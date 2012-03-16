// $Id: Newmark.cpp 575 2011-09-20 18:05:35Z unger3 $

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/ptr_container/serialize_ptr_map.hpp>
#endif // ENABLE_SERIALIZATION

# ifdef _OPENMP
#include <omp.h>
# endif

#include "nuto/math/SparseDirectSolverMUMPS.h"
#include "nuto/math/SparseDirectSolverMKLPardiso.h"

#include "nuto/mechanics/nodes/NodeBase.h"
#include "nuto/mechanics/groups/Group.h"
#include "nuto/mechanics/structures/StructureBase.h"
#include "nuto/mechanics/timeIntegration/Newmark.h"
#include "nuto/mechanics/timeIntegration/TimeIntegrationEnum.h"
#include "nuto/math/FullMatrix.h"
#include "nuto/math/SparseMatrixCSRGeneral.h"
#include "nuto/math/SparseMatrixCSRSymmetric.h"

//! @brief constructor
//! @param mDimension number of nodes
NuTo::ImplicitNewtonRaphson::ImplicitNewtonRaphson ()  : TimeIntegrationBase ()
{
	mMuDampingMass = 0.;
	mToleranceForce = 1e-6;
	mMaxNumIterations = 20;
	mMinLineSearchStep = 0.01;
	mBeta = 0.25;
	mGamma = 0.5;
	mInternalEnergy = 0.;
	mExternalEnergy = 0.;
	mKineticEnergy = 0.;
	mDampedEnergy = 0.;
}

//! @brief ... Info routine that prints general information about the object (detail according to verbose level)
void NuTo::ImplicitNewtonRaphson::Info()const
{
	TimeIntegrationBase::Info();
}

#ifdef ENABLE_SERIALIZATION
// serializes the class
template void NuTo::ImplicitNewtonRaphson::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::ImplicitNewtonRaphson::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::ImplicitNewtonRaphson::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::ImplicitNewtonRaphson::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::ImplicitNewtonRaphson::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::ImplicitNewtonRaphson::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::ImplicitNewtonRaphson::serialize(Archive & ar, const unsigned int version)
{
	#ifdef DEBUG_SERIALIZATION
	    mLogger << "start serialization of Newmark" << "\n";
	#endif
	    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(TimeIntegrationBase)
	       & BOOST_SERIALIZATION_NVP(mToleranceForce)
	       & BOOST_SERIALIZATION_NVP(mMaxNumIterations)
	       & BOOST_SERIALIZATION_NVP(mMinLineSearchStep)
	       & BOOST_SERIALIZATION_NVP(mBeta)
	       & BOOST_SERIALIZATION_NVP(mGamma)
	       & BOOST_SERIALIZATION_NVP(mInternalEnergy)
	       & BOOST_SERIALIZATION_NVP(mExternalEnergy)
	       & BOOST_SERIALIZATION_NVP(mKineticEnergy)
	       & BOOST_SERIALIZATION_NVP(mDampedEnergy);


    #ifdef DEBUG_SERIALIZATION
        mLogger << "finish serialization of Newmark" << "\n";
    #endif
}

#endif // ENABLE_SERIALIZATION


//! @brief perform the time integration
//! @param rStructure ... structure
//! @param rTimeDelta ... length of the simulation
NuTo::Error::eError NuTo::ImplicitNewtonRaphson::Solve(StructureBase& rStructure, double rTimeDelta)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
#ifdef _OPENMP
    double wstart = omp_get_wtime ( );
#endif
    start=clock();
#endif
    //allocate solver
    NuTo::SparseDirectSolverMUMPS mySolver;
    //NuTo::SparseDirectSolverMKLPardiso mySolver;
#ifdef SHOW_TIME
        mySolver.SetShowTime(rStructure.GetShowTime());
#endif
    try
    {
    	if (mMaxTimeStep==0)
    		throw MechanicsException("[NuTo::ImplicitNewtonRaphson::Solve] max time step is set to zero.");

    	//renumber dofs and build constraint matrix
    	rStructure.NodeBuildGlobalDofs();

        //calculate constraint matrix
    	NuTo::SparseMatrixCSRGeneral<double> CmatTmp;
        rStructure.ConstraintGetConstraintMatrixAfterGaussElimination(CmatTmp);
        NuTo::SparseMatrixCSRVector2General<double> Cmat(CmatTmp);
        SparseMatrixCSRVector2General<double> CmatT (Cmat.Transpose());
        FullMatrix<double> bRHS;

        // allocate space for stiffness matrix
        SparseMatrixCSRVector2General<double> stiffMatrix_jj(rStructure.GetNumActiveDofs(), rStructure.GetNumActiveDofs());
        SparseMatrixCSRVector2General<double> stiffMatrix_jk(rStructure.GetNumActiveDofs(), rStructure.GetNumDofs() - rStructure.GetNumActiveDofs());
        SparseMatrixCSRVector2General<double> stiffMatrix_kj(rStructure.GetNumDofs() - rStructure.GetNumActiveDofs(), rStructure.GetNumActiveDofs());
        SparseMatrixCSRVector2General<double> stiffMatrix_kk(rStructure.GetNumDofs() - rStructure.GetNumActiveDofs(), rStructure.GetNumDofs() - rStructure.GetNumActiveDofs());

        //calculate individual mass matrix (I was lazy here and have just used general matrices, but symmetric is certainly better here)
        NuTo::SparseMatrixCSRVector2General<double> massMatrix_jj,massMatrix_jk,massMatrix_kj,massMatrix_kk;
        if (this->IsDynamic())
        {
			massMatrix_jj.Resize(rStructure.GetNumActiveDofs(),rStructure.GetNumActiveDofs());
			massMatrix_jk.Resize(rStructure.GetNumActiveDofs(),rStructure.GetNumDofs() - rStructure.GetNumActiveDofs());
			massMatrix_kj.Resize(rStructure.GetNumDofs() - rStructure.GetNumActiveDofs(),rStructure.GetNumActiveDofs());
			massMatrix_kk.Resize(rStructure.GetNumDofs() - rStructure.GetNumActiveDofs(),rStructure.GetNumDofs() - rStructure.GetNumActiveDofs());
			rStructure.BuildGlobalCoefficientSubMatricesGeneral(NuTo::StructureBaseEnum::MASS,massMatrix_jj,massMatrix_jk,massMatrix_kj,massMatrix_kk);
        }

        //extract displacements, velocities and accelerations
        NuTo::FullMatrix<double> disp_j,disp_k, vel_j, vel_k, acc_j, acc_k, delta_disp_j, delta_disp_k;
        NuTo::FullMatrix<double> trial_disp_j,trial_disp_k, trial_vel_j, trial_vel_k, trial_acc_j, trial_acc_k;
        NuTo::FullMatrix<double> extForce_j, extForce_k, prevExtForce_j, prevExtForce_k;
        NuTo::FullMatrix<double> intForce_j(rStructure.GetNumActiveDofs(),1), intForce_k(rStructure.GetNumDofs() - rStructure.GetNumActiveDofs(),1),
        		                 prevIntForce_j(rStructure.GetNumActiveDofs(),1), prevIntForce_k(rStructure.GetNumDofs() - rStructure.GetNumActiveDofs(),1);
        NuTo::FullMatrix<double> residual_j, residual_k, residual_mod, prevResidual_j, prevResidual_k;

        //store last converged displacements, velocities and accelerations
        NuTo::FullMatrix<double> lastConverged_disp_j,lastConverged_disp_k, lastConverged_vel_j, lastConverged_vel_k, lastConverged_acc_j, lastConverged_acc_k;
        rStructure.NodeExtractDofValues(lastConverged_disp_j, lastConverged_disp_k);
		mInternalEnergy = rStructure.ElementTotalGetInternalEnergy();
        FullMatrix<double> energyHistory(1,5);
        energyHistory(0,0) = mInternalEnergy;
        energyHistory(0,4) = mInternalEnergy;
        if (this->IsDynamic())
        {
			rStructure.NodeExtractDofFirstTimeDerivativeValues(lastConverged_vel_j,lastConverged_vel_k);
			rStructure.NodeExtractDofSecondTimeDerivativeValues(lastConverged_acc_j,lastConverged_acc_k);

			mKineticEnergy = 0.5*(lastConverged_vel_j.Dot(massMatrix_jj*lastConverged_vel_j+massMatrix_jk*lastConverged_vel_k)+lastConverged_vel_k.Dot(massMatrix_kj*lastConverged_vel_j+massMatrix_kk*lastConverged_vel_k));
			mExternalEnergy = 0;
			mDampedEnergy = 0.;

			energyHistory(0,1) = mKineticEnergy;
	        energyHistory(0,2) = mDampedEnergy;
	        energyHistory(0,3) = mExternalEnergy;
	        energyHistory(0,4)+= mKineticEnergy ;
        }

        double curTime  = 0;

    	//apply constraints for last converged time step
    	ConstraintsCalculateRHSAndApply(rStructure, curTime);
    	rStructure.ConstraintGetRHSAfterGaussElimination(bRHS);
        rStructure.NodeMergeActiveDofValues(lastConverged_disp_j); //disp_k is internally calculated from the previous constraint matrix

    	//calculate internal force
        rStructure.BuildGlobalGradientInternalPotentialSubVectors(prevIntForce_j,prevIntForce_k);
        prevResidual_j = prevIntForce_j;
        prevResidual_k = prevIntForce_k;

        if (this->IsDynamic())
        {
			//add damping terme
        	prevResidual_j += (massMatrix_jj*lastConverged_vel_j+massMatrix_jk*lastConverged_vel_k) *mMuDampingMass;
        	prevResidual_k += (massMatrix_kj*lastConverged_vel_j+massMatrix_kk*lastConverged_vel_k) *mMuDampingMass;
			//add mass terme
			prevResidual_j += (massMatrix_jj*lastConverged_acc_j+massMatrix_jk*lastConverged_acc_k);
			prevResidual_k += (massMatrix_kj*lastConverged_acc_j+massMatrix_kk*lastConverged_acc_k);
        }
        //add external force
    	CalculateExternalLoad(rStructure, curTime, prevExtForce_j, prevExtForce_k);
    	prevResidual_j -= prevExtForce_j;
    	prevResidual_k -= prevExtForce_k;

        if (Cmat.GetNumEntries()>0)
        {
         	residual_mod=residual_j - CmatT*residual_k;
        }
        else
        {
        	residual_mod=residual_j;
        }

        if (residual_mod.Norm()>mToleranceForce)
        	throw MechanicsException("[NuTo::ImplicitNewtonRaphson::Solve] Initial configuration is not in (dynamic) equilibrium.");

        double timeStep = mMaxTimeStep;
        while (curTime < rTimeDelta)
        {
        	//apply constraints for last converged time step
        	ConstraintsCalculateRHSAndApply(rStructure, curTime);
        	rStructure.ConstraintGetRHSAfterGaussElimination(bRHS);
            rStructure.NodeMergeActiveDofValues(lastConverged_disp_j); //disp_k is internally calculated from the previous constraint matrix

            //add external force
        	CalculateExternalLoad(rStructure, curTime, prevExtForce_j, prevExtForce_k);

            //increase time step
        	curTime += timeStep;

        	//if the time difference towards the end is very small, increase the time step, if it exceeds the total time, decrease the time step
        	if (mAutomaticTimeStepping)
        	{
        		if (rTimeDelta-curTime<0.5*timeStep)
				{
					timeStep += rTimeDelta-curTime;
					curTime = rTimeDelta;
				}
        	}

            if (this->IsDynamic())
            {
				if (1.-mGamma*mMuDampingMass*timeStep<=0)
					throw MechanicsException("[NuTo::ImplicitNewtonRaphson::Solve] Factor for the mass matrix is negative - reduce your time step.");
            }

        	//calculate the initial out-of-balance force assuming for the zero accelerations for all dofs ->
            if (this->IsDynamic())
            {
				acc_j = (lastConverged_vel_j+lastConverged_acc_j*(timeStep*(0.5-mBeta)))*(-1./(timeStep*mBeta));
				acc_k = (lastConverged_vel_k+lastConverged_acc_k*(timeStep*(0.5-mBeta)))*(-1./(timeStep*mBeta));

				vel_j = lastConverged_vel_j+lastConverged_acc_j*((1.-mGamma)*timeStep)+acc_j*(mGamma*timeStep);
				vel_k = lastConverged_vel_k+lastConverged_acc_k*((1.-mGamma)*timeStep)+acc_k*(mGamma*timeStep);
            }
            disp_j = lastConverged_disp_j;
            disp_k = lastConverged_disp_k;

            rStructure.NodeMergeActiveDofValues(lastConverged_disp_j);

        	//calculate internal force
            rStructure.BuildGlobalGradientInternalPotentialSubVectors(prevIntForce_j,prevIntForce_k);
            residual_j = prevIntForce_j;
            residual_k = prevIntForce_k;
        	//calculate the initial out-of-balance force based on assuming for the zero accelerations for all dofs ->
            if (this->IsDynamic())
            {
				//add damping terme
				residual_j += (massMatrix_jj*vel_j+massMatrix_jk*vel_k) *mMuDampingMass;
				residual_k += (massMatrix_kj*vel_j+massMatrix_kk*vel_k) *mMuDampingMass;
				//add mass terme
				residual_j += (massMatrix_jj*acc_j+massMatrix_jk*acc_k);
				residual_k += (massMatrix_kj*acc_j+massMatrix_kk*acc_k);
            }
            //add external force
        	CalculateExternalLoad(rStructure, curTime, extForce_j, extForce_k);
            residual_j -= extForce_j;
            residual_k -= extForce_k;

        	//apply constraints for the new time step (modified bRHS)
        	ConstraintsCalculateRHSAndApply(rStructure, curTime);
        	FullMatrix<double> deltaBRHS(bRHS);
        	rStructure.ConstraintGetRHSAfterGaussElimination(bRHS);
        	deltaBRHS-=bRHS;
        	deltaBRHS*=-1.;

            //calculate stiffness and disp force vector (the displacements of the dependent dofs do not take into account the modified rhs)
            if (Cmat.GetNumEntries()>0)
            {
            	stiffMatrix_jj.SetZeroEntries();stiffMatrix_jk.SetZeroEntries();stiffMatrix_kj.SetZeroEntries();stiffMatrix_kk.SetZeroEntries();
            	rStructure.BuildGlobalCoefficientSubMatricesGeneral(NuTo::StructureBaseEnum::STIFFNESS, stiffMatrix_jj, stiffMatrix_jk, stiffMatrix_kj, stiffMatrix_kk);
                if (this->IsDynamic())
                {
					double factor((1.+mGamma*mMuDampingMass*timeStep)/(timeStep*timeStep*mBeta));
					stiffMatrix_jj.AddScal(massMatrix_jj,factor);
					stiffMatrix_jk.AddScal(massMatrix_jk,factor);
					stiffMatrix_kj.AddScal(massMatrix_kj,factor);
					stiffMatrix_kk.AddScal(massMatrix_kk,factor);
                }

                //add damping and mass to full hessian
                stiffMatrix_jj -= CmatT * stiffMatrix_kj + stiffMatrix_jk * Cmat;
                stiffMatrix_jj += CmatT * stiffMatrix_kk * Cmat;
            }
            else
            {
            	stiffMatrix_jj.SetZeroEntries();stiffMatrix_jk.SetZeroEntries();
                rStructure.BuildGlobalCoefficientSubMatricesGeneral(NuTo::StructureBaseEnum::STIFFNESS, stiffMatrix_jj, stiffMatrix_jk);

                //add damping and mass to full hessian
                if (this->IsDynamic())
                {
					double factor((1.+mGamma*mMuDampingMass*timeStep)/(timeStep*timeStep*mBeta));
					stiffMatrix_jj.AddScal(massMatrix_jj,factor);
					stiffMatrix_jk.AddScal(massMatrix_jk,factor);
                }
           }

            //calculate the residual for the zero time step
			residual_j += stiffMatrix_jk * deltaBRHS;
            if (Cmat.GetNumEntries()>0)
            {
    			residual_k += stiffMatrix_kk * deltaBRHS;
             	residual_mod=residual_j - CmatT*residual_k;
            }
            else
            {
            	residual_mod=residual_j;
            }


            //solve for trial state
            NuTo::SparseMatrixCSRGeneral<double> hessianModSolver(stiffMatrix_jj);
            hessianModSolver.SetOneBasedIndexing();
            mySolver.Solve(hessianModSolver, residual_mod, delta_disp_j);
            delta_disp_j*=-1;

            //calculate trial state
            disp_j += delta_disp_j;
        	delta_disp_k = bRHS - Cmat*disp_j - disp_k;
			disp_k += delta_disp_k;
            if (this->IsDynamic())
            {
				acc_j  += delta_disp_j * (1./(timeStep*timeStep*mBeta));
				vel_j  += delta_disp_j * (mGamma/(timeStep*mBeta));

				acc_k  += delta_disp_k * (1./(timeStep*timeStep*mBeta));
				vel_k  += delta_disp_k * (mGamma/(timeStep*mBeta));
            }

            //apply displacements
            rStructure.NodeMergeActiveDofValues(disp_j);

            //calculate internal force
            rStructure.BuildGlobalGradientInternalPotentialSubVectors(intForce_j, intForce_k);
            residual_j = intForce_j;
            residual_k = intForce_k;
            if (this->IsDynamic())
            {
				//add damping terme
				residual_j += (massMatrix_jj*vel_j+massMatrix_jk*vel_k) *mMuDampingMass;
				residual_k += (massMatrix_kj*vel_j+massMatrix_kk*vel_k) *mMuDampingMass;

				//add mass terme
				residual_j += massMatrix_jj*acc_j+massMatrix_jk*acc_k;
				residual_k += massMatrix_kj*acc_j+massMatrix_kk*acc_k;
            }

            //add external force
            residual_j -= extForce_j;
            residual_k -= extForce_k;

            //calculate the initial residual
            if (Cmat.GetNumEntries()>0)
            {
                residual_mod=residual_j - CmatT*residual_k;
            }
            else
            {
                residual_mod=residual_j;
            }

            //calculate norm of initial residual (first time step)
            double normResidual(residual_mod.Norm());

            int iteration(0);
            while(normResidual>mToleranceForce && iteration<mMaxNumIterations)
            {
                //calculate stiffness
                if (Cmat.GetNumEntries()>0)
                {
                	stiffMatrix_jj.SetZeroEntries();stiffMatrix_jk.SetZeroEntries();stiffMatrix_kj.SetZeroEntries();stiffMatrix_kk.SetZeroEntries();
                    rStructure.BuildGlobalCoefficientSubMatricesGeneral(NuTo::StructureBaseEnum::STIFFNESS, stiffMatrix_jj, stiffMatrix_jk, stiffMatrix_kj, stiffMatrix_kk);
                    if (this->IsDynamic())
                    {
						double factor((1.+mGamma*mMuDampingMass*timeStep)/(timeStep*timeStep*mBeta));
						stiffMatrix_jj.AddScal(massMatrix_jj,factor);
						stiffMatrix_jk.AddScal(massMatrix_jk,factor);
						stiffMatrix_kj.AddScal(massMatrix_kj,factor);
						stiffMatrix_kk.AddScal(massMatrix_kk,factor);
                    }

                    //add damping and mass to full hessian
                    stiffMatrix_jj -= CmatT * stiffMatrix_kj + stiffMatrix_jk * Cmat;
                    stiffMatrix_jj += CmatT * stiffMatrix_kk * Cmat;
                }
                else
                {
                	stiffMatrix_jj.SetZeroEntries();stiffMatrix_jk.SetZeroEntries();
                    rStructure.BuildGlobalCoefficientSubMatricesGeneral(NuTo::StructureBaseEnum::STIFFNESS, stiffMatrix_jj, stiffMatrix_jk);

                    if (this->IsDynamic())
                    {
						//add damping and mass to full hessian
						double factor((1.+mGamma*mMuDampingMass*timeStep)/(timeStep*timeStep*mBeta));
						stiffMatrix_jj.AddScal(massMatrix_jj,factor);
						stiffMatrix_jk.AddScal(massMatrix_jk,factor);
                    }
                }

                //solve for new state
                NuTo::SparseMatrixCSRGeneral<double> hessianModSolver(stiffMatrix_jj);
                hessianModSolver.SetOneBasedIndexing();
                mySolver.Solve(hessianModSolver, residual_mod, delta_disp_j);
                delta_disp_j*=-1;

                //perform a line search
                double alpha(1);
                double trialNormResidual(0);
               	do
                {
                     //calculate trial state
                    trial_disp_j = disp_j + delta_disp_j * alpha;
                    delta_disp_k = (Cmat*delta_disp_j)*(-1.);
                    trial_disp_k = disp_k + delta_disp_k * alpha;

                    if (this->IsDynamic())
                    {
						trial_acc_j  = acc_j+delta_disp_j * (alpha/(timeStep*timeStep*mBeta));
						trial_acc_k  = acc_k+delta_disp_k * (alpha/(timeStep*timeStep*mBeta));

						trial_vel_j  = vel_j+delta_disp_j * (alpha*mGamma/(timeStep*mBeta));
						trial_vel_k  = vel_k+delta_disp_k * (alpha*mGamma/(timeStep*mBeta));
                    }

                    //apply displacements
                    rStructure.NodeMergeActiveDofValues(trial_disp_j);

                    //calculate internal force
                    rStructure.BuildGlobalGradientInternalPotentialSubVectors(intForce_j, intForce_k);
                    residual_j = intForce_j;
                    residual_k = intForce_k;

                    if (this->IsDynamic())
                    {
						//add damping terme
						residual_j += (massMatrix_jj*trial_vel_j+massMatrix_jk*trial_vel_k) *mMuDampingMass;
						residual_k += (massMatrix_jk.TransMult(trial_vel_j)+massMatrix_kk*trial_vel_k) *mMuDampingMass;

						//add mass terme
						residual_j += (massMatrix_jj*trial_acc_j+massMatrix_jk*trial_acc_k);
						residual_k += (massMatrix_jk.TransMult(trial_acc_j)+massMatrix_kk*trial_acc_k);
                    }

                    //add external force (assumed to be independent of the deformation)
                 	residual_j -=extForce_j;
                	residual_k -=extForce_k;

                    //calculate the initial residual
                    if (Cmat.GetNumEntries()>0)
                    {
                        residual_mod = residual_j - CmatT*residual_k;
                    }
                    else
                    {
                        residual_mod = residual_j;
                    }

                    trialNormResidual=residual_mod.Norm();

                   	alpha*=0.5;
                }
                while(alpha>mMinLineSearchStep && trialNormResidual>(1.-alpha)*normResidual);
               	//in the first iteration, there is no line search

                if (alpha>mMinLineSearchStep)
                {
                	//improvement is achieved, go to next Newton step
                    disp_j = trial_disp_j;
                    disp_k = trial_disp_k;

                    if (this->IsDynamic())
                    {
						acc_j  = trial_acc_j;
						acc_k  = trial_acc_k;

						vel_j  = trial_vel_j;
						vel_k  = trial_vel_k;
                    }

                    normResidual = trialNormResidual;

                    iteration++;
                }
                else
                {
					//and leave
					iteration = mMaxNumIterations;
                }
            }
            // end of while(normResidual<mToleranceForce && iteration<mMaxNumIterations)
            if (normResidual<=mToleranceForce)
            {
            	//converged solution
                rStructure.ElementTotalUpdateStaticData();

                //calculate energies (at this point, use the trapezoidal rule for all energies, for some energies, you can compare that to the direct calculation
                // e.g. internal energy for elastic materials or the kinetic energy
                //double internalEnergyExact = rStructure.ElementTotalGetInternalEnergy();
                FullMatrix<double> delta_disp_j(disp_j-lastConverged_disp_j);
                FullMatrix<double> delta_disp_k(disp_k-lastConverged_disp_k);

                mInternalEnergy+= 0.5*(delta_disp_j.Dot(intForce_j+prevIntForce_j)+
                		               delta_disp_k.Dot(intForce_k+prevIntForce_k));

                //double kineticEnergyExact = 0.5*(vel_j.Dot(massMatrix_jj*vel_j+massMatrix_jk*vel_k)+vel_k.Dot(massMatrix_kj*vel_j+massMatrix_kk*vel_k));
                mKineticEnergy+= 0.5*(delta_disp_j.Dot(massMatrix_jj*(acc_j+lastConverged_acc_j)+massMatrix_jk*(acc_k+lastConverged_acc_k))+
                		          delta_disp_k.Dot(massMatrix_kj*(acc_j+lastConverged_acc_j)+massMatrix_kk*(acc_k+lastConverged_acc_k)));

                mDampedEnergy+=mMuDampingMass*0.5*(delta_disp_j.Dot(massMatrix_jj*(vel_j+lastConverged_vel_j)+massMatrix_jk*(vel_k+lastConverged_vel_k))+
                		                           delta_disp_k.Dot(massMatrix_kj*(vel_j+lastConverged_vel_j)+massMatrix_kk*(vel_k +lastConverged_vel_k)));

                mExternalEnergy += 0.5*(delta_disp_j.Dot(extForce_j+prevExtForce_j) + delta_disp_k.Dot(extForce_k+prevExtForce_k)+
                		                delta_disp_j.Dot(residual_j+prevResidual_j) + delta_disp_k.Dot(residual_k+prevResidual_k));

                NuTo::FullMatrix<double> energyVector(1,5);
                energyVector(0,0) = mInternalEnergy;
                energyVector(0,1) = mKineticEnergy;
                energyVector(0,2) = mDampedEnergy;
                energyVector(0,3) = mExternalEnergy;
                energyVector(0,4) = mInternalEnergy + mKineticEnergy + mDampedEnergy - mExternalEnergy;

                energyHistory.AppendRows(energyVector);


                //std::cout << "mInternalEnergy " << mInternalEnergy << "(exact " << internalEnergyExact << "), kinetic energy " << mKineticEnergy << "(exact " << kineticEnergyExact << "), damped energy " << mDampedEnergy ;
                //std::cout << ", external energy " << mExternalEnergy << "\n";
                //std::cout << "energy stored in the system (internal + kinetic) " << mInternalEnergy+mKineticEnergy << "\n";
                //std::cout << "energy removed from the system (damping - external) " << mDampedEnergy-mExternalEnergy << "\n";
                //std::cout << "total energy " << mInternalEnergy+mKineticEnergy + mDampedEnergy-mExternalEnergy << "\n";

                //store converged step
                lastConverged_disp_j = disp_j;
                lastConverged_disp_k = disp_k;
                if (this->IsDynamic())
                {
					lastConverged_vel_j = vel_j;
					lastConverged_vel_k = vel_k;
					lastConverged_acc_j = acc_j;
					lastConverged_acc_k = acc_k;
                }

                prevResidual_j = residual_j;
                prevResidual_k = residual_k;

                //update nodal data
                rStructure.NodeMergeActiveDofValues(disp_j);
                if (this->IsDynamic())
                {
					rStructure.NodeMergeDofFirstTimeDerivativeValues(vel_j,vel_k);
					rStructure.NodeMergeDofSecondTimeDerivativeValues(acc_j,acc_k);
                }
                //postprocess
                mTime+=timeStep;

                std::cout << "Convergence after " << iteration << " iterations at time " << mTime << "." << "\n";

                //sum the residuals for the required output nodes
                for (int countGroup=0; countGroup<mVecGroupNodesReactionForces.GetNumRows(); countGroup++)
                {
                	FullMatrix<double> reactionForce;
                	if (rStructure.GetDimension()==2)
						reactionForce.Resize(1,2);
					else
						reactionForce.Resize(1,3);
                	boost::ptr_map<int,GroupBase>::iterator itGroup = rStructure.mGroupMap.find(mVecGroupNodesReactionForces(countGroup,0));
                    if (itGroup==rStructure.mGroupMap.end())
                        throw MechanicsException("[NuTo::ImplicitNewtonRaphson::Solve] node group with the given identifier for the reaction forces does not exist.");
                    if (itGroup->second->GetType()!=NuTo::Groups::Nodes)
                    	throw MechanicsException("[NuTo::ImplicitNewtonRaphson::Solve] Group is not a node group (reaction forces).");
                    Group<NodeBase> *nodeGroup = itGroup->second->AsGroupNode();
                    assert(nodeGroup!=0);

                    //all nodes have to have the same dimension
                    if(nodeGroup->GetNumMembers()<1)
                    	throw MechanicsException("[NuTo::StructureBase::NodeGroupGetCoordinates] Group has no members.");

                    for (Group<NodeBase>::iterator itNode=nodeGroup->begin(); itNode!=nodeGroup->end();itNode++)
                    {
                    	for (int countDimension=0; countDimension<rStructure.GetDimension(); countDimension++)
                    	{
                    		int theDof = itNode->second->GetDofDisplacement(countDimension);
                    		if (theDof<rStructure.GetNumActiveDofs())
                    			reactionForce(0,countDimension)+=residual_j(theDof,0);
                    		else
                    			reactionForce(0,countDimension)+=residual_k(theDof-rStructure.GetNumActiveDofs(),0);
                    	}
                    }
                    energyVector.AppendColumns(reactionForce);
                }
                PostProcess(rStructure, energyVector);

                //eventually increase next time step
                if (iteration<0.25*mMaxNumIterations)
                {
                	timeStep*=1.5;
                	if (timeStep>mMaxTimeStep)
                		timeStep = mMaxTimeStep;
                }
            }
            else
            {
				//no convergence
            	if (mAutomaticTimeStepping)
            	{
					//no convergence, reduce the time step and start from scratch
					curTime -= timeStep;
					timeStep*=0.5;
				}
				else
				{
					throw MechanicsException("[NuTo::ImplicitNewtonRaphson::Solve] no convergence with the current maximum number of iterations, either use automatic time stepping, reduce the time step or the minimal line search cut back factor.");
				}
            }
        }
    }
    catch (MechanicsException& e)
    {
        e.AddMessage("[NuTo::ImplicitNewtonRaphson::Solve] performing Newton-Raphson iteration.");
        throw e;
    }
#ifdef SHOW_TIME
    end=clock();
#ifdef _OPENMP
    double wend = omp_get_wtime ( );
    if (mShowTime)
        rStructure.GetLogger()<<"[NuTo::ImplicitNewtonRaphson::Solve] " << difftime(end,start)/CLOCKS_PER_SEC << "sec(" << wend-wstart <<")\n";
#else
    if (mShowTime)
    	rStructure.GetLogger()<< "[NuTo::ImplicitNewtonRaphson::Solve] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << "\n";
#endif
#endif
    return NuTo::Error::SUCCESSFUL;

}


#ifdef ENABLE_SERIALIZATION
#ifndef SWIG
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::ImplicitNewtonRaphson)
#endif // SWIG
#endif // ENABLE_SERIALIZATION