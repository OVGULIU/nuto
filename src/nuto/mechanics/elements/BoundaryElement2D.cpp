/*
 * BoundaryElement2D.cpp
 *
 *  Created on: 9 Jun 2015
 *      Author: ttitsche
 */

#include "nuto/mechanics/elements/BoundaryElement2D.h"
#include "nuto/mechanics/elements/Element2D.h"
#include "nuto/mechanics/structures/StructureBase.h"
#include "nuto/mechanics/sections/SectionBase.h"

#include "nuto/mechanics/constitutive/ConstitutiveTangentLocal.h"
#include "nuto/mechanics/constitutive/ConstitutiveBase.h"
#include "nuto/mechanics/constitutive/moistureTransport/RelativeHumidity.h"
#include "nuto/mechanics/constitutive/moistureTransport/WaterVolumeFraction.h"
#include "nuto/mechanics/elements/ElementOutputBase.h"
#include "nuto/mechanics/elements/ElementDataBase.h"
#include "nuto/mechanics/constitutive/mechanics/DeformationGradient2D.h"

#include "nuto/mechanics/constitutive/mechanics/NonlocalEqStrain.h"
#include "nuto/mechanics/constitutive/mechanics/LocalEqStrain.h"

#include "nuto/mechanics/nodes/NodeBase.h"

NuTo::BoundaryElement2D::BoundaryElement2D(const ElementBase* rBaseElement, int rSurfaceId) :
        NuTo::BoundaryElementBase::BoundaryElementBase(rBaseElement, rSurfaceId)
{

}

NuTo::Error::eError NuTo::BoundaryElement2D::Evaluate(boost::ptr_multimap<NuTo::Element::eOutput, NuTo::ElementOutputBase>& rElementOutput)
{
    try
    {

        const SectionBase* section(GetSection());
        if (section == 0)
            throw MechanicsException("[NuTo::BoundaryElement2D::Evaluate] no section allocated for element.");

        const std::set<Node::eAttributes>& dofs = mInterpolationType->GetDofs();
        const std::set<Node::eAttributes>& activeDofs = mInterpolationType->GetActiveDofs();

        int numActiveDofs = mInterpolationType->GetNumActiveDofs();

        // extract all node values and store them
        std::map<Node::eAttributes, Eigen::MatrixXd> nodalValues;
        for (auto dof : dofs)
        {
            nodalValues[dof] = ExtractNodeValues(0, dof);
        }

        // Gradient damage model
        DeformationGradient2D deformationGradient;
        NonlocalEqStrain nonlocalEqStrain;
        LocalEqStrain localEqStrain;
        ConstitutiveTangentLocal<3, 1> tangentLocalEqStrainStrain;
        ConstitutiveTangentLocal<1, 1> nonlocalParameter;


        //allocate relative humidity
        RelativeHumidity relativeHumidity;
        RelativeHumidity relativeHumidityD1;

        //allocate water phase fraction
        WaterVolumeFraction waterVolumeFraction;
        WaterVolumeFraction waterVolumeFractionD1;


        ConstitutiveTangentLocal<1,1> residualBoundarySurfaceWaterPhase;
        ConstitutiveTangentLocal<1,1> residualBoundarySurfaceVaporPhase;
        ConstitutiveTangentLocal<1,1> tangentSurfaceRelativeHumidityTransportCoefficient;
        ConstitutiveTangentLocal<1,1> tangentSurfaceWaterVolumeFractionTransportCoefficient;

        //allocate input list and output list
        std::map<NuTo::Constitutive::Input::eInput, const ConstitutiveInputBase*> constitutiveInputList;
        std::map<NuTo::Constitutive::Output::eOutput, ConstitutiveOutputBase*> constitutiveOutputList;

        //define constitutive input list
        for (auto dof : dofs)
        {
            if (mInterpolationType->IsConstitutiveInput(dof) == false)
                continue;
            switch (dof)
            {
            case Node::NONLOCALEQSTRAIN:
                constitutiveInputList[NuTo::Constitutive::Input::DEFORMATION_GRADIENT_2D] = &deformationGradient;
                constitutiveInputList[NuTo::Constitutive::Input::NONLOCAL_EQ_STRAIN] = &(nonlocalEqStrain);
                break;
            case Node::RELATIVEHUMIDITY:
            {
                constitutiveInputList[NuTo::Constitutive::Input::RELATIVE_HUMIDITY]             = &relativeHumidity;
                constitutiveInputList[NuTo::Constitutive::Input::RELATIVE_HUMIDITY_D1]          = &relativeHumidityD1;
                break;
            }
            case Node::WATERVOLUMEFRACTION:
            {
                constitutiveInputList[NuTo::Constitutive::Input::WATER_VOLUME_FRACTION]         = &waterVolumeFraction;
                constitutiveInputList[NuTo::Constitutive::Input::WATER_VOLUME_FRACTION_D1]      = &waterVolumeFractionD1;
                break;
            }
            default:
                break;
            }
        }

        //define constitutive output list
        for (auto it = rElementOutput.begin(); it != rElementOutput.end(); it++)
        {
            switch (it->first)
            {
            case Element::INTERNAL_GRADIENT:
                it->second->GetFullVectorDouble().Resize(numActiveDofs);
                //if the stiffness matrix is constant, the corresponding internal force is calculated via the Kd
                //on the global level
                for (auto dof : activeDofs)
                {
                    switch (dof)
                    {
                    case Node::NONLOCALEQSTRAIN:
                        constitutiveOutputList[NuTo::Constitutive::Output::LOCAL_EQ_STRAIN] = &localEqStrain;
                        constitutiveOutputList[NuTo::Constitutive::Output::NONLOCAL_PARAMETER_XI] = &nonlocalParameter;
                        break;

                    case Node::RELATIVEHUMIDITY:
                    {
                        if (activeDofs.find(Node::WATERVOLUMEFRACTION) != activeDofs.end())
                        {
                            constitutiveOutputList[NuTo::Constitutive::Output::BOUNDARY_SURFACE_VAPOR_PHASE_RESIDUAL]  = &residualBoundarySurfaceVaporPhase;
                        }
                        break;
                    }
                    case Node::WATERVOLUMEFRACTION:
                    {
                        if (activeDofs.find(Node::RELATIVEHUMIDITY) != activeDofs.end())
                        {
                            constitutiveOutputList[NuTo::Constitutive::Output::BOUNDARY_SURFACE_WATER_PHASE_RESIDUAL]  = &residualBoundarySurfaceWaterPhase;
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
                break;
            case Element::HESSIAN_0_TIME_DERIVATIVE:
            {
                it->second->GetFullMatrixDouble().Resize(numActiveDofs, numActiveDofs);
                it->second->GetFullMatrixDouble().setZero();
                it->second->SetSymmetry(true);
                it->second->SetConstant(true);
                for (auto dof : activeDofs)
                {
                    switch (dof)
                    {
                    case Node::NONLOCALEQSTRAIN:
                        constitutiveOutputList[NuTo::Constitutive::Output::LOCAL_EQ_STRAIN] = &localEqStrain;
                        constitutiveOutputList[NuTo::Constitutive::Output::D_LOCAL_EQ_STRAIN_XI_D_STRAIN_2D] = &tangentLocalEqStrainStrain;
                        constitutiveOutputList[NuTo::Constitutive::Output::NONLOCAL_PARAMETER_XI] = &nonlocalParameter;
                        break;
                    case Node::RELATIVEHUMIDITY:
                    {
                        if (activeDofs.find(Node::WATERVOLUMEFRACTION) != activeDofs.end())
                        {
                            constitutiveOutputList[NuTo::Constitutive::Output::BOUNDARY_SURFACE_RELATIVE_HUMIDIY_TRANSPORT_COEFFICIENT]  = &tangentSurfaceRelativeHumidityTransportCoefficient;
                        }
                        break;
                    }
                    case Node::WATERVOLUMEFRACTION:
                    {
                        if (activeDofs.find(Node::RELATIVEHUMIDITY) != activeDofs.end())
                        {
                            constitutiveOutputList[NuTo::Constitutive::Output::BOUNDARY_SURFACE_WATER_VOLUME_FRACTION_TRANSPORT_COEFFICIENT]  = &tangentSurfaceWaterVolumeFractionTransportCoefficient;
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
                break;
            case Element::HESSIAN_1_TIME_DERIVATIVE:
                it->second->GetFullMatrixDouble().Resize(numActiveDofs, numActiveDofs);
                it->second->SetSymmetry(true);
                it->second->SetConstant(true);
                break;
            case Element::HESSIAN_2_TIME_DERIVATIVE:
            {
                it->second->GetFullMatrixDouble().Resize(numActiveDofs, numActiveDofs);
                it->second->SetSymmetry(true);
                it->second->SetConstant(true);
                break;
            }
            case Element::UPDATE_STATIC_DATA:
                constitutiveOutputList[NuTo::Constitutive::Output::UPDATE_STATIC_DATA] = 0;
                break;
            case Element::GLOBAL_ROW_DOF:
            {
                const Eigen::VectorXi& globalRowDofsEigen = mBaseElement->AsElement2D()->CalculateGlobalRowDofs();
                std::vector<int> globalRowDofsStd(globalRowDofsEigen.data(), globalRowDofsEigen.data() + globalRowDofsEigen.rows());
                it->second->GetVectorInt() = globalRowDofsStd;
            }
                break;
            case Element::GLOBAL_COLUMN_DOF:
            {
                const Eigen::VectorXi& globalColumnDofsEigen = mBaseElement->AsElement2D()->CalculateGlobalColumnDofs();
                std::vector<int> globalColumnDofsStd(globalColumnDofsEigen.data(), globalColumnDofsEigen.data() + globalColumnDofsEigen.rows());
                it->second->GetVectorInt() = globalColumnDofsStd;
            }
                break;
            default:
                break;
            }
        }               //end for: constitutive output list

        std::map<Node::eAttributes, Eigen::VectorXd> shapeFunctions;
        std::map<Node::eAttributes, Eigen::MatrixXd> derivativeShapeFunctions;

        Eigen::MatrixXd derivativeShapeFunctionsNatural;
        Eigen::MatrixXd derivativeNaturalSurfaceCoordinates;


        Eigen::Matrix<double, 1, 1> ipCoordsSurface;
        Eigen::Matrix<double, 2, 1> ipCoordsNatural;

        Eigen::Matrix2d jacobian;
        Eigen::Matrix2d invJacobian;

        double factor;

        // loop over surface integration points
        for (int theIp = 0; theIp < GetNumIntegrationPoints(); theIp++)
        {

            double tmp;
            GetIntegrationType()->GetLocalIntegrationPointCoordinates1D(theIp, tmp);
            ipCoordsSurface(0) = tmp;
            ipCoordsNatural = mInterpolationType->Get(Node::COORDINATES).CalculateNaturalSurfaceCoordinates(ipCoordsSurface, mSurfaceId);

            // #######################################
            // ##  Calculate the surface jacobian
            // ## = || [dX / dXi] * [dXi / dAlpha] ||
            // #######################################
            derivativeShapeFunctionsNatural = mInterpolationType->Get(Node::COORDINATES).CalculateDerivativeShapeFunctionsNatural(ipCoordsNatural);
            // = [dX / dXi]
            jacobian = nodalValues[Node::COORDINATES] * derivativeShapeFunctionsNatural;
            invJacobian = jacobian.inverse();

            // = [dXi / dAlpha]
            derivativeNaturalSurfaceCoordinates = mInterpolationType->Get(Node::COORDINATES).CalculateDerivativeNaturalSurfaceCoordinates(ipCoordsSurface, mSurfaceId);
            // = || [dX / dXi] * [dXi / dAlpha] ||
            double detJacobian = (jacobian * derivativeNaturalSurfaceCoordinates).norm();
            factor = section->GetThickness() * detJacobian;


            for (auto dof : dofs)
            {
                if (dof == Node::COORDINATES)
                    continue;
                const InterpolationBase& interpolationType = mInterpolationType->Get(dof);
                shapeFunctions[dof] = interpolationType.CalculateShapeFunctions(ipCoordsNatural);
                // this lazy product here is so much faster than any other implementation via a seperate method
                // possibly due to more efficient mallocs
                derivativeShapeFunctions[dof] = interpolationType.CalculateDerivativeShapeFunctionsNatural(ipCoordsNatural).lazyProduct(invJacobian);
            }

            // define constitutive inputs
            for (auto dof : dofs)
            {
                if (mInterpolationType->IsConstitutiveInput(dof) == false)
                    continue;
                switch (dof)
                {
                case Node::DISPLACEMENTS:
                    deformationGradient = mBaseElement->AsElement2D()->CalculateDeformationGradient(derivativeShapeFunctions.at(dof), nodalValues.at(dof));
                    break;
                case Node::NONLOCALEQSTRAIN:
                    nonlocalEqStrain(0, 0) = (nodalValues[dof] * shapeFunctions[dof])(0, 0);
                    break;
                case Node::RELATIVEHUMIDITY:
                {
                    relativeHumidity(0,0)               = (nodalValues[Node::RELATIVEHUMIDITY] * shapeFunctions[Node::RELATIVEHUMIDITY])(0,0);
                    relativeHumidityD1(0,0)             = (ExtractNodeValues(1, Node::RELATIVEHUMIDITY) * shapeFunctions[Node::RELATIVEHUMIDITY])(0,0);

                }
                    break;
                case Node::WATERVOLUMEFRACTION:
                {
                    waterVolumeFraction(0,0)            = (nodalValues[Node::WATERVOLUMEFRACTION] * shapeFunctions[Node::WATERVOLUMEFRACTION])(0,0);
                    waterVolumeFractionD1(0,0)          = (ExtractNodeValues(1, Node::WATERVOLUMEFRACTION) * shapeFunctions[Node::WATERVOLUMEFRACTION])(0);


                }
                    break;
                default:
                    break;
                }
            }



            ConstitutiveBase* constitutivePtr = GetConstitutiveLaw(theIp);
            try
            {
                Error::eError error = constitutivePtr->Evaluate2D(this, theIp, constitutiveInputList, constitutiveOutputList);
                if (error != Error::SUCCESSFUL)
                    return error;
            } catch (NuTo::MechanicsException &e)
            {
                e.AddMessage("[NuTo::BoundaryElement2D::Evaluate] error evaluating the constitutive model.");
                throw e;
            }


            //        std::cout << "Nonlocal parameter " << nonlocalParameter.GetValue(0) << std::endl;

            int numDofsNonlocalEqStrain = 0;
            NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic> KkkMod;
            BoundaryType::eType currentBCType = mBoundaryConditionType;
            double alpha = 0.;

            if (activeDofs.find(Node::NONLOCALEQSTRAIN) != activeDofs.end())
            {
                numDofsNonlocalEqStrain = mInterpolationType->Get(Node::NONLOCALEQSTRAIN).GetNumDofs();
                KkkMod.Resize(numDofsNonlocalEqStrain, numDofsNonlocalEqStrain);

                alpha = std::sqrt(nonlocalParameter.GetValue(0));

                //            std::cout << "local eq strain    " << localEqStrain[0] << std::endl;
                //            std::cout << "nonlocal eq strain " << nonlocalEqStrain[0] << std::endl;

                double e0 = constitutivePtr->GetParameterDouble(Constitutive::eConstitutiveParameter::TENSILE_STRENGTH) /
                            constitutivePtr->GetParameterDouble(NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS);
                if (mBoundaryConditionType == BoundaryType::MACAULAY)
                {

                    // determine state ...
                    bool switchToNeumann = (localEqStrain[0] > nonlocalEqStrain[0]) and (localEqStrain[0] > e0);

                    // ... and use existing implementations
                    if (switchToNeumann)
                    {
                        currentBCType = BoundaryType::NEUMANN_HOMOGENEOUS;
                        std::cout << "Macaulay Culcin helps out in element " << mBaseElement->GetStructure()->ElementGetId(this) << std::endl;

                    }
                    else
                        currentBCType = BoundaryType::ROBIN_INHOMOGENEOUS;
                }

                switch (currentBCType)
                {
                case BoundaryType::NOT_SET:
                    throw MechanicsException("[NuTo::BoundaryElement2D::Evaluate] Boundary condition type not set! ");
                    break;
                case BoundaryType::ROBIN_INHOMOGENEOUS:
                    KkkMod = shapeFunctions[Node::NONLOCALEQSTRAIN] * shapeFunctions[Node::NONLOCALEQSTRAIN].transpose() * factor / alpha;
                    break;
                default:
                    break;
                }

            }

            //calculate output
            for (auto it = rElementOutput.begin(); it != rElementOutput.end(); it++)
            {
                switch (it->first)
                {
                case Element::INTERNAL_GRADIENT:
                {
                    //if the stiffness matrix is constant, the corresponding internal force is calculated via the Kd
                    //on the global level
                    for (auto dof : activeDofs)
                    {
                        int startIndex = mInterpolationType->Get(dof).GetLocalStartIndex();
                        switch (dof)
                        {
                        case Node::NONLOCALEQSTRAIN:
                        {
                            // add Nt c eqStrain
                            NuTo::FullVector<double, Eigen::Dynamic>& f = it->second->GetFullVectorDouble();
                            if (currentBCType == BoundaryType::ROBIN_INHOMOGENEOUS)
                            {
                                f.block(startIndex, 0, numDofsNonlocalEqStrain, 1) += shapeFunctions[Node::NONLOCALEQSTRAIN] * factor / alpha * (nonlocalEqStrain[0] - localEqStrain[0]);
                            }
                            break;
                        }
                        case Node::RELATIVEHUMIDITY:
                        {
                            if(activeDofs.find(Node::WATERVOLUMEFRACTION) != activeDofs.end())
                            {
                                mBaseElement-> AsElement2D() -> AddDetJNtX( shapeFunctions.at(Node::RELATIVEHUMIDITY),
                                                                            residualBoundarySurfaceVaporPhase,
                                                                            factor,
                                                                            startIndex,
                                                                            it->second->GetFullVectorDouble());
                            }
                            break;
                        }
                        case Node::WATERVOLUMEFRACTION:
                        {
                            if(activeDofs.find(Node::RELATIVEHUMIDITY) != activeDofs.end())
                            {
                                mBaseElement-> AsElement2D() -> AddDetJNtX( shapeFunctions.at(Node::WATERVOLUMEFRACTION),
                                                                            residualBoundarySurfaceWaterPhase,
                                                                            factor,
                                                                            startIndex,
                                                                            it->second->GetFullVectorDouble());
                            }
                            break;
                        }
                        default:
                            break;
                        }
                    }
                }
                    break;
                case Element::HESSIAN_0_TIME_DERIVATIVE:
                {
                    for (auto dof : activeDofs)
                    {
                        int startIndex = mInterpolationType->Get(dof).GetLocalStartIndex();
                        switch (dof)
                        {
                        case Node::NONLOCALEQSTRAIN:
                            // add the modified Kkk to the element output
                            it->second->GetFullMatrixDouble().AddBlock(startIndex, startIndex, KkkMod);
                            if (currentBCType == BoundaryType::ROBIN_INHOMOGENEOUS)
                            {
                                int dStart = mInterpolationType->Get(Node::DISPLACEMENTS).GetLocalStartIndex();
                                int eStart = startIndex;

                                mBaseElement->AsElement2D()->AddDetJNtdLocalEqStraindEpsilonB(
                                        shapeFunctions.at(Node::NONLOCALEQSTRAIN),
                                        tangentLocalEqStrainStrain,
                                        derivativeShapeFunctions.at(Node::DISPLACEMENTS),
                                        factor,
                                        eStart, dStart, it->second->GetFullMatrixDouble());
                            }
                            break;
                        case Node::RELATIVEHUMIDITY:
                        {
                            if(activeDofs.find(Node::WATERVOLUMEFRACTION) != activeDofs.end())
                            {
                                auto RelHumShapeFunction = shapeFunctions.at(Node::RELATIVEHUMIDITY);

                                mBaseElement-> AsElement2D() -> AddDetJNtXN(RelHumShapeFunction,
                                                                            RelHumShapeFunction,
                                                                            tangentSurfaceRelativeHumidityTransportCoefficient,
                                                                            factor,
                                                                            startIndex,
                                                                            startIndex,
                                                                            it->second->GetFullMatrixDouble());
                            }
                            break;
                        }
                        case Node::WATERVOLUMEFRACTION:
                        {
                            if(activeDofs.find(Node::RELATIVEHUMIDITY) != activeDofs.end())
                            {
                                auto WatVolShapeFunction = shapeFunctions.at(Node::WATERVOLUMEFRACTION);

                                mBaseElement-> AsElement2D() -> AddDetJNtXN(WatVolShapeFunction,
                                                                            WatVolShapeFunction,
                                                                            tangentSurfaceWaterVolumeFractionTransportCoefficient,
                                                                            factor,
                                                                            startIndex,
                                                                            startIndex,
                                                                            it->second->GetFullMatrixDouble());
                            }
                            break;
                        }
                        default:
                            break;
                        }
                    }
                }
                    break;
                default:
                    break;
                }
            }
        }

    } catch (NuTo::MechanicsException& e)
    {
        std::stringstream ss;
        ss << mBaseElement->GetStructure()->ElementGetId(this);
        e.AddMessage("[NuTo::BoundaryElement2D::Evaluate] Error evaluating element data of element " + ss.str() + ".");
        throw e;
    }
    return Error::SUCCESSFUL;

}
NuTo::ConstitutiveStaticDataBase* NuTo::BoundaryElement2D::AllocateStaticData(const ConstitutiveBase* rConstitutiveLaw) const
{
    return rConstitutiveLaw->AllocateStaticDataEngineeringStress_EngineeringStrain2D(this);
}

//! @brief cast the base pointer to an BoundaryElement2D, otherwise throws an exception
const NuTo::BoundaryElement2D* NuTo::BoundaryElement2D::AsBoundaryElement2D() const
{
    return this;
}

//! @brief cast the base pointer to an BoundaryElement2D, otherwise throws an exception
NuTo::BoundaryElement2D* NuTo::BoundaryElement2D::AsBoundaryElement2D()
{
    return this;
}

int NuTo::BoundaryElement2D::GetNumNodes() const
{
    return GetBoundaryNodeIndices().rows();
}


int NuTo::BoundaryElement2D::GetBoundaryNodeIndex(int rBoundaryNodeIndex) const
{
    return GetBoundaryNodeIndices()[rBoundaryNodeIndex];
}

const Eigen::VectorXi NuTo::BoundaryElement2D::GetBoundaryNodeIndices() const
{
    // temporarily use a std::vector for its push_back() method
    std::vector<int> boundaryNodeIndices;

    const InterpolationType& it = *(mBaseElement->GetInterpolationType());
    const Eigen::VectorXi surfaceNodeIds = it.GetSurfaceNodeIndices(mSurfaceId);
    assert(surfaceNodeIds.rows() == 2);

    // get A and B as two points on the boundary
    const Eigen::VectorXd& A = it.GetNaturalNodeCoordinates(surfaceNodeIds.at(0,0));
    const Eigen::VectorXd& B = it.GetNaturalNodeCoordinates(surfaceNodeIds.at(1,0));

    // check every node of the element if its natural coordinates are inbetween those of A and B
    int numNodes = it.GetNumNodes();
    for (int iNode = 0; iNode < numNodes; ++iNode)
    {
        const Eigen::VectorXd& P = it.GetNaturalNodeCoordinates(iNode);
        if (PointIsOnBoundary(A,B,P))
            boundaryNodeIndices.push_back(iNode);
    }

    // map back to Eigen::VectorXi as return value
    return Eigen::Map<Eigen::VectorXi>(boundaryNodeIndices.data(), boundaryNodeIndices.size());
}

bool NuTo::BoundaryElement2D::PointIsOnBoundary(const Eigen::VectorXd rA, const Eigen::VectorXd rB, const Eigen::VectorXd rP) const
{
    assert(rA.rows() == 2);
    assert(rB.rows() == 2);
    assert(rP.rows() == 2);
//    Eigen::Matrix2d matr;
//    matr.block<2,1>(0,0) = (rB - rA).block<2,1>(0,0);
//    matr.block<2,1>(0,1) = (rA - rP).block<2,1>(0,0);
//
//    double det = matr.determinant();

    double det = ( (rB(0)-rA(0))  * (rA(1)-rP(1)) )  - ( (rA(0)-rP(0)) * (rB(1)-rA(1)) );

    return std::abs(det) < 1.e-10;
}

bool NuTo::BoundaryElement2D::IsBoundaryConditionFulfilled() const
{

    int id = mStructure->ElementGetId(this);
    if (id != 3300 and id != 3301 and id != 3302)
        return false;

    std::cout << std::endl;
    std::cout << "[NuTo::BoundaryElement2D::CalculateStuffOnBoundary] ####################### " << std::endl;
    std::cout << "Calculating stuff of elementId " << GetStructure()->ElementGetId(this) << std::endl;


    const std::set<Node::eAttributes>& dofs = mInterpolationType->GetDofs();
    const std::set<Node::eAttributes>& activeDofs = mInterpolationType->GetActiveDofs();

    // extract all node values and store them
    std::map<Node::eAttributes, Eigen::MatrixXd> nodalValues;
    for (auto dof : dofs)
    {
        nodalValues[dof] = ExtractNodeValues(0, dof);
    }



    // calculate some other points as linear combination of both
    int numPoints = 5;
    for (int i = 0; i < numPoints; ++i)
    {
        Eigen::VectorXd boundaryPoint(1);
        boundaryPoint(0) = -1 + 2.*i/(numPoints-1.);

        Eigen::VectorXd ipCoordsNatural = mInterpolationType->Get(Node::COORDINATES).CalculateNaturalSurfaceCoordinates(boundaryPoint, mSurfaceId);
        Eigen::Vector3d globalCoordinates = InterpolateDof3D(ipCoordsNatural, Node::COORDINATES);

        std::cout << "Calculating stuff of Point " << globalCoordinates.transpose() << std::endl;


        // calculate normal vector

        // #######################################
        // ##  Calculate the surface jacobian
        // ## = || [dX / dXi] * [dXi / dAlpha] ||
        // #######################################
        Eigen::MatrixXd derivativeShapeFunctionsNatural = mInterpolationType->Get(Node::COORDINATES).CalculateDerivativeShapeFunctionsNatural(ipCoordsNatural);
        Eigen::MatrixXd shapeFunctions = mInterpolationType->Get(Node::COORDINATES).CalculateShapeFunctions(ipCoordsNatural);

        // = [dX / dXi]
        Eigen::Matrix2d jacobian = nodalValues[Node::COORDINATES] * derivativeShapeFunctionsNatural;
        Eigen::Matrix2d invJacobian = jacobian.inverse();

        // = [dXi / dAlpha]
        Eigen::MatrixXd derivativeNaturalSurfaceCoordinates = mInterpolationType->Get(Node::COORDINATES).CalculateDerivativeNaturalSurfaceCoordinates(boundaryPoint, mSurfaceId);
        // = || [dX / dXi] * [dXi / dAlpha] ||
        Eigen::VectorXd surfaceTangentVector = jacobian * derivativeNaturalSurfaceCoordinates;
        Eigen::VectorXd surfaceNormalVector(2);
        surfaceNormalVector[0] = surfaceTangentVector[1];
        surfaceNormalVector[1] = - surfaceTangentVector[0];
        surfaceNormalVector.normalize();

//        std::cout << "Surface normal vector: n = (" << surfaceNormalVector.transpose() << ")^T" << std::endl;

        // calculate B
        const InterpolationBase& interpolationType = mInterpolationType->Get(Node::NONLOCALEQSTRAIN);
        Eigen::MatrixXd derivativeShapeFunctions = interpolationType.CalculateDerivativeShapeFunctionsNatural(ipCoordsNatural).lazyProduct(invJacobian);

        // calculate grad eeq
        Eigen::MatrixXd gradEeq = nodalValues[Node::NONLOCALEQSTRAIN] * derivativeShapeFunctions;
        std::cout << "grad eeq: \n" << gradEeq << std::endl;
        double nTgradEeq = (gradEeq * surfaceNormalVector).at(0,0);
        std::cout << "n.T * grad eeq: " << nTgradEeq << std::endl;
        //

    }

    return false;


}
#ifdef ENABLE_VISUALIZE
void NuTo::BoundaryElement2D::Visualize(VisualizeUnstructuredGrid& rVisualize, const boost::ptr_list<NuTo::VisualizeComponentBase>& rWhat)
{
    /*
    rVisualize.DefinePointDataScalar("gradeeq");

    const std::set<Node::eAttributes>& dofs = mInterpolationType->GetDofs();
    const std::set<Node::eAttributes>& activeDofs = mInterpolationType->GetActiveDofs();

    // extract all node values and store them
    std::map<Node::eAttributes, Eigen::MatrixXd> nodalValues;
    for (auto dof : dofs)
    {
        nodalValues[dof] = ExtractNodeValues(0, dof);
    }

    Eigen::Matrix<double, 1, 1> ipCoordsSurface;
    Eigen::Matrix<double, 2, 1> ipCoordsNatural;

    Eigen::VectorXd result = Eigen::VectorXd::Zero(mInterpolationType->Get(Node::NONLOCALEQSTRAIN).GetNumDofs());

    for (int iIp = 0; iIp  < GetNumIntegrationPoints(); ++iIp )
    {
        double tmp;
        GetIntegrationType()->GetLocalIntegrationPointCoordinates1D(iIp, tmp);
        ipCoordsSurface(0) = tmp;
        ipCoordsNatural = mInterpolationType->Get(Node::COORDINATES).CalculateNaturalSurfaceCoordinates(ipCoordsSurface, mSurfaceId);

//        Eigen::Vector3d globalCoordinates = InterpolateDof3D(ipCoordsNatural, Node::COORDINATES);

        Eigen::VectorXd shapeFunctions = mInterpolationType->Get(Node::COORDINATES).CalculateShapeFunctions(ipCoordsNatural);

        // #######################################
        // ##  Calculate the surface jacobian
        // ## = || [dX / dXi] * [dXi / dAlpha] ||
        // #######################################
        Eigen::MatrixXd derivativeShapeFunctionsNatural = mInterpolationType->Get(Node::COORDINATES).CalculateDerivativeShapeFunctionsNatural(ipCoordsNatural);
        // = [dX / dXi]
        Eigen::Matrix2d jacobian = nodalValues[Node::COORDINATES] * derivativeShapeFunctionsNatural;
        Eigen::Matrix2d invJacobian = jacobian.inverse();

        // = [dXi / dAlpha]
        Eigen::MatrixXd derivativeNaturalSurfaceCoordinates = mInterpolationType->Get(Node::COORDINATES).CalculateDerivativeNaturalSurfaceCoordinates(ipCoordsSurface, mSurfaceId);
        // = || [dX / dXi] * [dXi / dAlpha] ||
        Eigen::VectorXd surfaceTangentVector = jacobian * derivativeNaturalSurfaceCoordinates;
        Eigen::VectorXd surfaceNormalVector(2);
        surfaceNormalVector[0] = surfaceTangentVector[1];
        surfaceNormalVector[1] = - surfaceTangentVector[0];
        surfaceNormalVector.normalize();

//        std::cout << "Surface normal vector: n = (" << surfaceNormalVector.transpose() << ")^T" << std::endl;

        // calculate B
        const InterpolationBase& interpolationType = mInterpolationType->Get(Node::NONLOCALEQSTRAIN);
        Eigen::MatrixXd derivativeShapeFunctions = interpolationType.CalculateDerivativeShapeFunctionsNatural(ipCoordsNatural).lazyProduct(invJacobian);

        // calculate grad eeq
        Eigen::RowVector3d gradEeq = Eigen::RowVector3d::Zero();
        gradEeq.block<1,2>(0,0)= nodalValues[Node::NONLOCALEQSTRAIN] * derivativeShapeFunctions;

        double normalComponennt = surfaceNormalVector.dot(gradEeq.block<1,2>(0,0));


        result += shapeFunctions*normalComponennt;
    }

//    for (int iNode = 0; iNode < mBaseElement->GetNumNodes(Node::NONLOCALEQSTRAIN); ++iNode)
//    {
//        const NodeBase* node = mBaseElement->GetNode(iNode, Node::NONLOCALEQSTRAIN);
//        // get 3D coords
//        Eigen::Vector3d coords = Eigen::Vector3d::Zero();
//        coords.block<1,2>(0,0) = node->GetCoordinates2D();
//
//        unsigned int pointId = rVisualize.AddPoint(coords.data());
//        rVisualize.SetPointDataScalar(pointId, "gradeeq", result.at(iNode,0));
//
//    }

*/


}
#endif // ENABLE_VISUALIZE
