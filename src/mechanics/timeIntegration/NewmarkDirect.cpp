#include "base/CallbackInterface.h"
#include "base/Timer.h"
#include "mechanics/structures/StructureOutputBlockMatrix.h"
#include "mechanics/structures/StructureBase.h"
#include "mechanics/timeIntegration/ResultGroupNodeForce.h"
#include "mechanics/constitutive/ConstitutiveEnum.h"
#include "mechanics/constitutive/inputoutput/ConstitutiveCalculateStaticData.h"
#include "mechanics/constitutive/inputoutput/ConstitutiveIOMap.h"
#include "mechanics/constitutive/inputoutput/ConstitutiveScalar.h"
#include "mechanics/structures/StructureBaseEnum.h"
#include "mechanics/timeIntegration/NewmarkDirect.h"
#include "math/SparseMatrix.h"
#include "math/SparseMatrixCSRVector2General.h"
#include "mechanics/structures/Assembler.h"

using namespace NuTo;

NewmarkDirect::NewmarkDirect(StructureBase* rStructure)
    : NewmarkBase(rStructure)
    , mHessian2(rStructure->GetDofStatus())
{
}


void NewmarkDirect::Solve(double rTimeDelta)
{
    Timer timerFull(__FUNCTION__, mStructure->GetShowTime(), mStructure->GetLogger());

    mStructure->NodeBuildGlobalDofs(__PRETTY_FUNCTION__);
    auto dofValues = InitialState();

    if (mAutomaticTimeStepping)
    {
        // the minimal time step is equivalent to six cut-backs
        SetMinTimeStep(mMinTimeStep > 0. ? mMinTimeStep : mTimeStep * std::pow(0.5, 6.));
    }

    while (mTimeObject.GetCurrentTime() < rTimeDelta)
    {
        // LEAVE IT OR FIX IT IF YOU CAN:
        // If you dont make a copy of the dof set with dofStatus.GetDofTypes() and use it directly in the for loop
        // everything seems to work fine, but valgrind tells you otherwise. Problem is, that the DofType is changed
        // during the function call inside the loop which leads to reads in already freed blocks of memory
        std::set<Node::eDof> currentDofSet = mStructure->GetDofStatus().GetDofTypes();
        for (const auto& dof : currentDofSet)
        {
            mStructure->DofTypeSetIsActive(dof, true);
        }

        if (mTimeObject.GetTimestep() < mMinTimeStep)
            throw MechanicsException(__PRETTY_FUNCTION__,
                                     "Time step is smaller than minimum - no convergence was reached.");

        const auto bRHS = UpdateAndGetAndMergeConstraintRHS(mTimeObject.GetCurrentTime(), dofValues[0]);
        const auto prevExtForce = CalculateCurrentExternalLoad(mTimeObject.GetCurrentTime());

        mTimeObject.Proceed();

        const auto deltaBRHS = UpdateAndGetConstraintRHS(mTimeObject.GetCurrentTime()) - bRHS;

        IterateForActiveDofValues(prevExtForce, deltaBRHS, dofValues);
    }
}


std::pair<int, BlockScalar> NewmarkDirect::FindEquilibrium(StructureOutputBlockVector& structureResidual,
                                                           const StructureOutputBlockVector& extForce,
                                                           StructureOutputBlockVector& delta_dof_dt0,
                                                           std::array<StructureOutputBlockVector, 3>& dof_dt,
                                                           const BlockSparseMatrix& constraintMatrix, double timeStep)
{
    const auto& dofStatus = mStructure->GetDofStatus();
    auto residual = Assembler::ApplyCMatrix(structureResidual, constraintMatrix);
    BlockScalar residualNorm = residual.CalculateInfNorm();

    int iteration = 0;
    while (!(residualNorm < mToleranceResidual) && iteration < mMaxNumIterations)
    {
        auto hessians = EvaluateHessians();

        delta_dof_dt0.J = BuildHessianModAndSolveSystem(hessians, residual, timeStep);
        delta_dof_dt0.K = constraintMatrix * delta_dof_dt0.J * (-1.);
        ++mIterationCount;

        double alpha = 1;
        BlockScalar trialNormResidual(dofStatus);
        StructureOutputBlockVector trial_dof_dt0(dofStatus, true);
        StructureOutputBlockVector trial_dof_dt1(dofStatus, true);
        StructureOutputBlockVector trial_dof_dt2(dofStatus, true);

        // perform a line search
        do
        {
            trial_dof_dt0 = dof_dt[0] + delta_dof_dt0 * alpha;
            if (mStructure->GetNumTimeDerivatives() >= 1)
                trial_dof_dt1 = dof_dt[1] + delta_dof_dt0 * (alpha * mGamma / (timeStep * mBeta));
            if (mStructure->GetNumTimeDerivatives() >= 2)
                trial_dof_dt2 = dof_dt[2] + delta_dof_dt0 * (alpha / (timeStep * timeStep * mBeta));

            MergeDofValues(trial_dof_dt0, trial_dof_dt1, trial_dof_dt2, false);

            const auto intForce = EvaluateInternalGradient();

            structureResidual = CalculateResidual(intForce, extForce, hessians[2], trial_dof_dt1, trial_dof_dt2);
            residual = Assembler::ApplyCMatrix(structureResidual, constraintMatrix);

            trialNormResidual = residual.CalculateInfNorm();

            mStructure->GetLogger() << "[Linesearch a=" << std::to_string(alpha).substr(0, 6)
                                    << "] Trial residual: " << trialNormResidual << "\n";

            alpha *= 0.5;

        } while (mPerformLineSearch && alpha > mMinLineSearchStep && trialNormResidual > (1. - alpha) * residualNorm);

        if (alpha > mMinLineSearchStep || !mPerformLineSearch)
        {
            // improvement is achieved, go to next Newton step
            dof_dt[0] = trial_dof_dt0;
            if (mStructure->GetNumTimeDerivatives() >= 1)
                dof_dt[1] = trial_dof_dt1;
            if (mStructure->GetNumTimeDerivatives() >= 2)
                dof_dt[2] = trial_dof_dt2;
            residualNorm = trialNormResidual;

            PrintInfoIteration(residualNorm, iteration);
            iteration++;
        }
        else
            iteration = mMaxNumIterations;
    }
    return std::make_pair(iteration, residualNorm);
}


StructureOutputBlockMatrix NewmarkDirect::CalculateMuDampingMatrix(const StructureOutputBlockMatrix& hessian2) const
{
    if (mUseMuDamping and mStructure->GetNumTimeDerivatives() < 2)
        throw MechanicsException(__PRETTY_FUNCTION__, "MuDampingMass requires a mass matrix (2nd time derivatives).");

    const auto& dofStatus = mStructure->GetDofStatus();
    StructureOutputBlockMatrix hessian1(dofStatus);
    hessian1.Resize(dofStatus.GetNumActiveDofsMap(), dofStatus.GetNumDependentDofsMap());
    hessian1.SetZero();
    hessian1.AddScal(hessian2, mMuDampingMass);
    return hessian1;
}

StructureOutputBlockVector NewmarkDirect::CalculateDof1(const StructureOutputBlockVector& rDeltaDof_dt0,
                                                        const StructureOutputBlockVector& rDof_dt1,
                                                        const StructureOutputBlockVector& rDof_dt2,
                                                        double rTimeStep) const
{
    return rDeltaDof_dt0 * (mGamma / (rTimeStep * mBeta)) + rDof_dt1 * (1. - mGamma / mBeta) +
           rDof_dt2 * (rTimeStep * (1. - mGamma / (2. * mBeta)));
}


StructureOutputBlockVector NewmarkDirect::CalculateDof2(const StructureOutputBlockVector& rDeltaDof_dt0,
                                                        const StructureOutputBlockVector& rDof_dt1,
                                                        const StructureOutputBlockVector& rDof_dt2,
                                                        double rTimeStep) const
{
    return rDeltaDof_dt0 * (1. / (rTimeStep * rTimeStep * mBeta)) - rDof_dt1 * (1. / (rTimeStep * mBeta)) -
           rDof_dt2 * ((0.5 - mBeta) / mBeta);
}


StructureOutputBlockVector NewmarkDirect::CalculateResidual(const StructureOutputBlockVector& rIntForce,
                                                            const StructureOutputBlockVector& rExtForce,
                                                            const StructureOutputBlockMatrix& rHessian_dt2,
                                                            const StructureOutputBlockVector& rDof_dt1,
                                                            const StructureOutputBlockVector& rDof_dt2) const
{
    StructureOutputBlockVector residual = rExtForce - rIntForce;

    // The residual for numTimeDerivatives = 1 is included in the internal forces.
    // If there is muDamping, there must be a rHessian2.
    if (mStructure->GetNumTimeDerivatives() >= 2)
    {
        residual -= rHessian_dt2 * (rDof_dt1 * mMuDampingMass + rDof_dt2);
    }
    return residual;
}


void NewmarkDirect::CalculateResidualKForPostprocessing(StructureOutputBlockVector& rResidual,
                                                        const StructureOutputBlockMatrix& rHessian_dt2,
                                                        const StructureOutputBlockVector& rDof_dt1,
                                                        const StructureOutputBlockVector& rDof_dt2) const
{
    if (mStructure->GetDofStatus().HasInteractingConstraints())
        return; // in this case, residual.K is needed for the calculation of residual mod and it is already calculated.

    bool hasNodeForce = false;
    for (const auto& it : mResultMap)
        if (it.second->GetResultType() == eTimeIntegrationResultType::GROUP_NODE_FORCE)
        {
            hasNodeForce = true;
            break; // exit loop
        }
    if (hasNodeForce && mStructure->GetNumTimeDerivatives() >= 2)
    {
        const auto dof = rDof_dt1 * mMuDampingMass + rDof_dt2;
        rResidual.K -= rHessian_dt2.KJ * dof.J + rHessian_dt2.KK * dof.K;
    }
    // else:  no need to calculate forces if they are not needed in the post processing
}


void NewmarkDirect::CalculateResidualTrial(StructureOutputBlockVector& rResidual,
                                           const BlockFullVector<double>& rDeltaBRHS,
                                           const std::array<StructureOutputBlockMatrix, 3>& hessians,
                                           const StructureOutputBlockVector& rDof_dt1,
                                           const StructureOutputBlockVector& rDof_dt2, double rTimeStep) const
{
    StructureOutputBlockVector deltaDof_dt0(mStructure->GetDofStatus(), true);
    deltaDof_dt0.J.SetZero();
    deltaDof_dt0.K = rDeltaBRHS;

    rResidual -= hessians[0] * deltaDof_dt0;

    if (mStructure->GetNumTimeDerivatives() >= 1)
    {
        StructureOutputBlockVector delta_dof1 = CalculateDof1(deltaDof_dt0, rDof_dt1, rDof_dt2, rTimeStep) - rDof_dt1;
        rResidual -= hessians[1] * delta_dof1;
    }

    if (mStructure->GetNumTimeDerivatives() >= 2)
    {
        StructureOutputBlockVector delta_dof2 = CalculateDof2(deltaDof_dt0, rDof_dt1, rDof_dt2, rTimeStep) - rDof_dt2;
        rResidual -= hessians[2] * delta_dof2;
    }
}


void NewmarkDirect::PrintInfoStagger() const
{
    if (mStepActiveDofs.size() == 1) // equals unstaggered solution, no info needed
        return;

    Logger& logger = mStructure->GetLogger();
    logger << "\n"
           << "Activated Dofs: | ";
    for (auto dof : mStructure->GetDofStatus().GetActiveDofTypes())
    {
        logger << Node::DofToString(dof) << " | ";
    }
    logger << "\n";
}


void NewmarkDirect::PrintInfoIteration(const BlockScalar& residualNorm, int iteration) const
{
    Logger& logger = mStructure->GetLogger();
    switch (mVerboseLevel)
    {
    case 0:
    {
        if (iteration == 0)
        {
            logger << "Iteration:";
        }
        else
        {
            logger << "*";
        }
        break;
    }
    case 1:
    {
        logger << "Iteration: " << iteration << "\n";
        for (const auto dof : mStructure->GetDofStatus().GetActiveDofTypes())
        {
            logger << "Residual " << Node::DofToString(dof) << ": " << residualNorm[dof] << "\n";
        }
        logger << "--------------------------\n";
        break;
    }
    default:
        break;
    }
}


std::array<StructureOutputBlockVector, 3> NuTo::NewmarkDirect::InitialState()
{
    CalculateStaticAndTimeDependentExternalLoad();

    mToleranceResidual.DefineDefaultValueToIninitializedDofTypes(mToleranceForce);

    if (mMaxTimeStep == 0)
        throw MechanicsException(__PRETTY_FUNCTION__, "Maximum time step is set to zero.");

    if (mStepActiveDofs.empty())
        mStepActiveDofs.push_back(mStructure->DofTypesGetActive());
    else
    {
        for (const auto& activeDofs : mStepActiveDofs)
        {
            if (activeDofs.empty())
                throw MechanicsException(__PRETTY_FUNCTION__, "Calculation step has no active DOFs.");
        }
    }

    if (mStructure->GetNumTimeDerivatives() >= 2)
    {
        if (mUseLumpedMass)
            mHessian2 = mStructure->BuildGlobalHessian2Lumped();
        else
            mHessian2 = mStructure->BuildGlobalHessian2();

        mDampingMatrix = CalculateMuDampingMatrix(mHessian2);
    }

    auto dofValues = ExtractDofValues();

    UpdateAndGetAndMergeConstraintRHS(mTimeObject.GetCurrentTime(), dofValues[0]);

    const auto gradientAndHessians = EvaluateGradientAndHessians();
    const auto intForce = gradientAndHessians.first;
    const auto hessians = gradientAndHessians.second;

    const auto initialExtForce = CalculateCurrentExternalLoad(mTimeObject.GetCurrentTime());

    auto residual = CalculateResidual(intForce, initialExtForce, hessians[2], dofValues[1], dofValues[2]);
    const auto& constraintMatrix = mStructure->GetAssembler().GetConstraintMatrix();
    const auto residual_mod = Assembler::ApplyCMatrix(residual, constraintMatrix);

    if (mToleranceResidual < residual_mod.CalculateInfNorm())
    {
        mStructure->GetLogger() << residual_mod.CalculateInfNorm();
        throw MechanicsException(__PRETTY_FUNCTION__, "Initial configuration is not in (dynamic) equilibrium.");
    }
    CalculateResidualKForPostprocessing(residual, hessians[2], dofValues[1], dofValues[2]);
    PostProcess(residual);
    return dofValues;
}


ConstitutiveInputMap NuTo::NewmarkDirect::CreateInputMap()
{
    ConstitutiveInputMap inputMap;
    inputMap[Constitutive::eInput::CALCULATE_STATIC_DATA] =
            std::make_unique<ConstitutiveCalculateStaticData>(eCalculateStaticData::EULER_BACKWARD);
    inputMap.emplace(Constitutive::eInput::TIME, std::make_unique<ConstitutiveScalar>());

    double& inputTime = (*inputMap.find(Constitutive::eInput::TIME)->second)[0];
    inputTime = mTimeObject.GetCurrentTime();
    return inputMap;
}


StructureOutputBlockVector NuTo::NewmarkDirect::EvaluateInternalGradient()
{
    const auto& dofStatus = mStructure->GetDofStatus();
    StructureOutputMap evalInternalGradient;
    StructureOutputBlockVector intForce(dofStatus, true);
    evalInternalGradient[eStructureOutput::INTERNAL_GRADIENT] = &intForce;

    auto inputMap = CreateInputMap();
    mStructure->Evaluate(inputMap, evalInternalGradient);
    return intForce;
}


std::array<StructureOutputBlockMatrix, 3> NuTo::NewmarkDirect::EvaluateHessians()
{
    const auto& dofStatus = mStructure->GetDofStatus();
    std::array<StructureOutputBlockMatrix, 3> hessians = {StructureOutputBlockMatrix(dofStatus, true),
                                                          StructureOutputBlockMatrix(dofStatus), mHessian2};
    StructureOutputMap evalHessians;
    evalHessians[eStructureOutput::HESSIAN0] = &hessians[0];
    if (mStructure->GetNumTimeDerivatives() >= 1)
    {
        if (mUseMuDamping)
            hessians[1] = mDampingMatrix;
        else
        {
            hessians[1].Resize(dofStatus.GetNumActiveDofsMap(), dofStatus.GetNumDependentDofsMap());
            evalHessians[eStructureOutput::HESSIAN1] = &hessians[1];
        }
    }

    auto inputMap = CreateInputMap();
    mStructure->Evaluate(inputMap, evalHessians);
    if (mCheckCoefficientMatrix)
        mStructure->ElementCheckHessian0(1.e-6, 1.e-8);

    return hessians;
}


std::pair<StructureOutputBlockVector, std::array<StructureOutputBlockMatrix, 3>>
NuTo::NewmarkDirect::EvaluateGradientAndHessians()
{
    const auto& dofStatus = mStructure->GetDofStatus();
    StructureOutputBlockVector intForce(dofStatus, true);
    std::array<StructureOutputBlockMatrix, 3> hessians = {StructureOutputBlockMatrix(dofStatus, true),
                                                          StructureOutputBlockMatrix(dofStatus), mHessian2};
    StructureOutputMap evalGradAndHessians;
    evalGradAndHessians[eStructureOutput::HESSIAN0] = &hessians[0];
    evalGradAndHessians[eStructureOutput::INTERNAL_GRADIENT] = &intForce;
    if (mStructure->GetNumTimeDerivatives() >= 1)
    {
        if (mUseMuDamping)
            hessians[1] = mDampingMatrix;
        else
        {
            hessians[1].Resize(dofStatus.GetNumActiveDofsMap(), dofStatus.GetNumDependentDofsMap());
            evalGradAndHessians[eStructureOutput::HESSIAN1] = &hessians[1];
        }
    }

    auto inputMap = CreateInputMap();
    mStructure->Evaluate(inputMap, evalGradAndHessians);
    return std::make_pair(intForce, hessians);
}


void NewmarkDirect::IterateForActiveDofValues(const StructureOutputBlockVector& prevExtForce,
                                              const BlockFullVector<double>& deltaBRHS,
                                              std::array<StructureOutputBlockVector, 3>& lastConverged_dof_dt)
{
    // at the moment needed to do the postprocessing after the last step and not after every step of a staggered
    // solution.
    unsigned int staggeredStepNumber = 0;
    const auto& dofStatus = mStructure->GetDofStatus();
    // [0] = disp. , [1] = vel. , [2] = acc.
    std::array<StructureOutputBlockVector, 3> dof_dt = {StructureOutputBlockVector(dofStatus, true),
                                                        StructureOutputBlockVector(dofStatus, true),
                                                        StructureOutputBlockVector(dofStatus, true)};
    StructureOutputBlockVector delta_dof_dt0(dofStatus, true);
    const auto& constraintMatrix = mStructure->GetAssembler().GetConstraintMatrix();
    for (const auto& activeDofs : mStepActiveDofs)
    {
        ++staggeredStepNumber;
        mStructure->DofTypeSetIsActive(activeDofs);

        PrintInfoStagger();

        auto hessians = EvaluateHessians();
        const auto extForce = CalculateCurrentExternalLoad(mTimeObject.GetCurrentTime());
        auto residual = extForce - prevExtForce;
        CalculateResidualTrial(residual, deltaBRHS, hessians, lastConverged_dof_dt[1], lastConverged_dof_dt[2],
                               mTimeObject.GetTimestep());
        const auto residual_mod = Assembler::ApplyCMatrix(residual, constraintMatrix);

        mStructure->GetLogger() << "\n"
                                << "Initial trial residual:               " << residual_mod.CalculateInfNorm() << "\n";

        delta_dof_dt0.J = BuildHessianModAndSolveSystem(hessians, residual_mod, mTimeObject.GetTimestep());
        delta_dof_dt0.K = deltaBRHS - constraintMatrix * delta_dof_dt0.J;
        ++mIterationCount;

        // calculate trial state
        dof_dt[0] = lastConverged_dof_dt[0] + delta_dof_dt0;
        if (mStructure->GetNumTimeDerivatives() >= 1)
            dof_dt[1] = CalculateDof1(delta_dof_dt0, lastConverged_dof_dt[1], lastConverged_dof_dt[2],
                                      mTimeObject.GetTimestep());
        if (mStructure->GetNumTimeDerivatives() >= 2)
            dof_dt[2] = CalculateDof2(delta_dof_dt0, lastConverged_dof_dt[1], lastConverged_dof_dt[2],
                                      mTimeObject.GetTimestep());

        MergeDofValues(dof_dt[0], dof_dt[1], dof_dt[2], false);

        auto intForce = EvaluateInternalGradient();
        residual = CalculateResidual(intForce, extForce, hessians[2], dof_dt[1], dof_dt[2]);

        const auto result =
                FindEquilibrium(residual, extForce, delta_dof_dt0, dof_dt, constraintMatrix, mTimeObject.GetTimestep());

        const auto iterations = result.first;
        const auto residualNorm = result.second;

        if (residualNorm < mToleranceResidual)
        {
            mStructure->ElementTotalUpdateStaticData();

            // store converged step
            lastConverged_dof_dt[0] = dof_dt[0];
            if (mStructure->GetNumTimeDerivatives() >= 1)
                lastConverged_dof_dt[1] = dof_dt[1];
            if (mStructure->GetNumTimeDerivatives() >= 1)
                lastConverged_dof_dt[2] = dof_dt[2];

            auto prevResidual = residual;

            MergeDofValues(dof_dt[0], dof_dt[1], dof_dt[2], true);

            mTime += mTimeObject.GetTimestep();

            mStructure->GetLogger() << "Convergence after " << iterations << " iterations at time " << mTime
                                    << " (timestep " << mTimeObject.GetTimestep() << ").\n";
            mStructure->GetLogger() << "Residual: \t" << residualNorm << "\n";
            // perform Postprocessing
            if (staggeredStepNumber >= mStepActiveDofs.size())
            {
                CalculateResidualKForPostprocessing(prevResidual, hessians[2], dof_dt[1], dof_dt[2]);
                PostProcess(prevResidual);
            }

            // eventually increase next time step
            if (mAutomaticTimeStepping && iterations < 0.25 * mMaxNumIterations)
            {
                mTimeObject.ScaleTimestep(1.5);
            }

            if (mCallback && mCallback->Exit(*mStructure))
                return;
        }
        else
        {
            mStructure->GetLogger() << "No convergence with timestep " << mTimeObject.GetTimestep() << " at time "
                                    << mTime << "\n";
            if (mAutomaticTimeStepping)
            {
                // no convergence, reduce the time step and start from scratch
                mTimeObject.RestorePreviosTime();
                mTimeObject.ScaleTimestep(0.5);
            }
            else
            {
                throw MechanicsException(__PRETTY_FUNCTION__, "No convergence with the current maximum number of "
                                                              "iterations, either use automatic time stepping, "
                                                              "reduce the time step or the minimal line search cut "
                                                              "back factor.");
            }
        } // if tolerance
    } // active dof loop
}


BlockFullVector<double>
        NewmarkDirect::BuildHessianModAndSolveSystem(std::array<StructureOutputBlockMatrix, 3>& rHessians,
                                                     const BlockFullVector<double>& rResidualMod,
                                                     double rTimeStep) const
{
    Timer timer(__FUNCTION__, GetShowTime(), mStructure->GetLogger());

    // since rHessian0 will change in the next iteration, the rHessian0 will be the hessian for the solver
    if (mStructure->GetNumTimeDerivatives() >= 1)
        rHessians[0].AddScal(rHessians[1], mGamma / (mBeta * rTimeStep));

    if (mStructure->GetNumTimeDerivatives() >= 2)
        rHessians[0].AddScal(rHessians[2], 1. / (mBeta * rTimeStep * rTimeStep));

    rHessians[0].ApplyCMatrix(mStructure->GetAssembler().GetConstraintMatrix());

    return mSolver->Solve(rHessians[0].JJ, rResidualMod);
}
