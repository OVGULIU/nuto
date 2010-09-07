// $Id: ConstraintNodeGroupDisplacements2D.cpp 265 2010-06-08 08:47:00Z arnold2 $
#include <math.h>

#include "nuto/math/FullMatrix.h"
#include "nuto/math/SparseMatrixCSRGeneral.h"
#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/nodes/NodeCoordinatesDisplacements2D.h"
#include "nuto/mechanics/nodes/NodeDisplacements2D.h"
#include "nuto/mechanics/groups/Group.h"
#include "nuto/mechanics/constraints/ConstraintDisplacementsPeriodic2D.h"
#include "nuto/mechanics/structures/StructureBase.h"

//! @brief constructor
NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D(const StructureBase* rStructure, double rAngle, NuTo::FullMatrix<double> rStrain,
        const Group<NodeBase>* rGroupTop,const Group<NodeBase>* rGroupBottom,
        const Group<NodeBase>* rGroupLeft, const Group<NodeBase>* rGroupRight) :  ConstraintBase()
{
    mStructure = rStructure,
    mAngle = rAngle;
    if (mAngle>225)
        mAngle-=180;
    if (mAngle<45)
        mAngle+=180;
    if (mAngle<45 || mAngle>225)
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] angle should be in the range 0..360, which is reduced to 45..225 due to symmetries.");
    if (rStrain.GetNumRows()!=3 || rStrain.GetNumColumns()!=1)
        throw MechanicsException("[NuTo::ConstraintNodeDisplacementsPeriodic2D::ConstraintNodeDisplacementsPeriodic2D] the strain is matrix (3,1) with (e_xx, e_yy, gamma_xy)");
    mStrain[0] = rStrain(0,0);
    mStrain[1] = rStrain(1,0);
    mStrain[2] = rStrain(2,0);

    mGroupTop = rGroupTop;
    mGroupBottom = rGroupBottom;
    mGroupLeft = rGroupLeft;
    mGroupRight = rGroupRight;

    //determine the corner nodes
    Group<NodeBase>* newGroup = dynamic_cast<Group<NodeBase>*>(rGroupTop->Intersection (rGroupLeft));
    if (newGroup->GetNumMembers()!=1)
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] Top left corner node can not be determined.");
    mLeftUpperCorner = *(newGroup->begin());
    delete newGroup;

    newGroup = dynamic_cast<Group<NodeBase>*>(rGroupTop->Intersection (rGroupRight));
    if (newGroup->GetNumMembers()!=1)
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] Top right corner node can not be determined.");
    mRightUpperCorner = *(newGroup->begin());
    delete newGroup;

    newGroup = dynamic_cast<Group<NodeBase>*>(rGroupBottom->Intersection (rGroupLeft));
    if (newGroup->GetNumMembers()!=1)
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] Bottom left corner node can not be determined.");
    mLeftLowerCorner = *(newGroup->begin());
    delete newGroup;

    newGroup = dynamic_cast<Group<NodeBase>*>(rGroupBottom->Intersection (rGroupRight));
    if (newGroup->GetNumMembers()!=1)
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] Bottom right corner node can not be determined.");
    mRightLowerCorner = *(newGroup->begin());
    delete newGroup;

    //get box coordinates
    double LeftUpperCoordinates[2], RightUpperCoordinates[2], LeftLowerCoordinates[2], RightLowerCoordinates[2];
    if (mLeftUpperCorner->GetNumCoordinates()!=2)
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] Upper left node does not have 2 coordinates.");
    mLeftUpperCorner->GetCoordinates2D(LeftUpperCoordinates);

    if (mRightUpperCorner->GetNumCoordinates()!=2)
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] Upper right node does not have 2 coordinates.");
    mRightUpperCorner->GetCoordinates2D(RightUpperCoordinates);

    if (mLeftLowerCorner->GetNumCoordinates()!=2)
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] Lower left node does not have 2 coordinates.");
    mLeftLowerCorner->GetCoordinates2D(LeftLowerCoordinates);

    if (mRightLowerCorner->GetNumCoordinates()!=2)
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] Lower right node does not have 2 coordinates.");
    mRightLowerCorner->GetCoordinates2D(RightLowerCoordinates);

    std::cout << "Left upper corner is node " << mStructure->NodeGetId(mLeftUpperCorner) << std::endl;
    std::cout << "Left lower corner is node " << mStructure->NodeGetId(mLeftLowerCorner) << std::endl;
    std::cout << "Right upper corner is node " << mStructure->NodeGetId(mRightUpperCorner) << std::endl;
    std::cout << "Right lower corner is node " << mStructure->NodeGetId(mRightLowerCorner) << std::endl;

    //check box
    if (LeftUpperCoordinates[0]!=LeftLowerCoordinates[0])
    {
        std::cout << "LeftUpperCoordinates " << LeftUpperCoordinates[0] << " " << LeftUpperCoordinates[1] << std::endl;
        std::cout << "LeftLowerCoordinates " << LeftLowerCoordinates[0] << " " << LeftLowerCoordinates[1] << std::endl;
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] Left boundary is not correct - check your node groups.");
    }
    if (RightUpperCoordinates[0]!=RightLowerCoordinates[0])
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] Right boundary is not correct - check your node groups.");

    if (RightUpperCoordinates[0]<=LeftUpperCoordinates[0])
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] left boundary coordinate is larger than right boundary coordinate.");

    if (LeftUpperCoordinates[1]!=RightUpperCoordinates[1])
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] upper boundary is not correct - check your node groups.");

    if (LeftLowerCoordinates[0]!=RightLowerCoordinates[1])
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] lower boundary is not correct - check your node groups.");

    if (LeftUpperCoordinates[1]<=LeftLowerCoordinates[1])
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::ConstraintDisplacementsPeriodic2D] upper boundary coordinate is larger than lower boundary coordinate.");
    SetBoundaryVectors();
}

//!@brief set the angle of the periodic boundary conditions
//!@param rRHS new right hand side
void NuTo::ConstraintDisplacementsPeriodic2D::SetAngle(double rAngle)
{
    mAngle = rAngle;
   SetBoundaryVectors();
}


//!@brief set the strain of the periodic boundary conditions
//!@param rStrain strain (e_xx,e_yy,gamma_xy)
void NuTo::ConstraintDisplacementsPeriodic2D::SetStrain(const NuTo::FullMatrix<double>& rStrain)
{
    if (rStrain.GetNumRows()!=3 || rStrain.GetNumColumns()!=1)
        throw MechanicsException("[NuTo::ConstraintNodeDisplacementsPeriodic2D::ConstraintNodeDisplacementsPeriodic2D] the strain is matrix (3,1) with (e_xx, e_yy, gamma_xy)");
    mStrain[0] = rStrain(0,0);
    mStrain[1] = rStrain(1,0);
    mStrain[2] = rStrain(2,0);
}

#define PI 3.14159265359

//!@brief calculate the border vectors in counterclockwise direction
void NuTo::ConstraintDisplacementsPeriodic2D::SetBoundaryVectors()
{
    double LeftUpperCoordinates[2], LeftLowerCoordinates[2];
    mLeftUpperCorner->GetCoordinates2D(LeftUpperCoordinates);
    mLeftLowerCorner->GetCoordinates2D(LeftLowerCoordinates);

    //calculate length of specimen
    double length = LeftUpperCoordinates[1]-LeftLowerCoordinates[1];
    std::cout << "length " << length << std::endl;
    double crackShift;
    if (mAngle!=90)
        crackShift = length/tan(mAngle*PI/180.);
    else
        crackShift=0.;

    if (mAngle>=45 && mAngle<135)
    {
        double crackPosX = (length-crackShift)*0.5;

        //calculate master nodes left boundary (green)
        mMasterNodesLeftBoundary.resize(0);
        mMasterNodesLeftBoundary.reserve(mGroupLeft->GetNumMembers());
        std::cout << "number of members left group " <<  mGroupLeft->GetNumMembers() << std::endl;
        for (Group<NodeBase>::iterator itNode=mGroupLeft->begin(); itNode!=mGroupLeft->end();itNode++)
        {
            mMasterNodesLeftBoundary.push_back(dynamic_cast<NodeCoordinatesDisplacements2D*>(*itNode));
        }
        sort(mMasterNodesLeftBoundary.begin(), mMasterNodesLeftBoundary.end(), less_YCoordinate2D(0.,length));

        //calculate master nodes bottom boundary (yellow-orange-blue)
        mMasterNodesBottomBoundary.resize(0);
        mMasterNodesBottomBoundary.reserve(mGroupBottom->GetNumMembers()-1);
        for (Group<NodeBase>::iterator itNode=mGroupBottom->begin(); itNode!=mGroupBottom->end();itNode++)
        {
            if ((*itNode)!=mRightLowerCorner)
                mMasterNodesBottomBoundary.push_back(dynamic_cast<NodeCoordinatesDisplacements2D*>(*itNode));

        }
        sort(mMasterNodesBottomBoundary.begin(), mMasterNodesBottomBoundary.end(), less_XCoordinate2D(crackPosX,length));

        //calculate slave nodes right boundary (green)
        mSlaveNodesRightBoundary.resize(0);
        mSlaveNodesRightBoundary.reserve(mGroupRight->GetNumMembers());
        for (Group<NodeBase>::iterator itNode=mGroupRight->begin(); itNode!=mGroupRight->end();itNode++)
        {
            mSlaveNodesRightBoundary.push_back(dynamic_cast<NodeCoordinatesDisplacements2D*>(*itNode));
        }
        sort(mSlaveNodesRightBoundary.begin(), mSlaveNodesRightBoundary.end(), less_YCoordinate2D(0.,length));

        //calculate slave nodes top boundary
        mSlaveNodesTopBoundary.resize(0);
        mSlaveNodesTopBoundary.reserve(mGroupTop->GetNumMembers()-1);
        for (Group<NodeBase>::iterator itNode=mGroupTop->begin(); itNode!=mGroupTop->end();itNode++)
        {
            if ((*itNode)!=mLeftUpperCorner)
                mSlaveNodesTopBoundary.push_back(dynamic_cast<NodeCoordinatesDisplacements2D*>(*itNode));
        }
        sort(mSlaveNodesTopBoundary.begin(), mSlaveNodesTopBoundary.end(), less_XCoordinate2D(length-crackPosX,length));
    }
    else
    {
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::SetBoundaryVectors] Not implemented for angles outside 0-90 degrees");
        if (mAngle>=135 && mAngle<225)
        {

        }
        else
        {
            throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::SetBoundaryVectors] angle should be between 45 and 225 degrees.");
        }
    }

    //Info about the nodes
    std::cout << "Constraint Info" << std::endl;
    for (int countBoundary=0; countBoundary<4; countBoundary++)
    {
        std::vector<NodeCoordinatesDisplacements2D*>* nodeVectorPtr;
        switch(countBoundary)
        {
        case 0:
            nodeVectorPtr = &mMasterNodesLeftBoundary;
            std::cout << "mMasterNodesLeftBoundary " << std::endl;
            break;
        case 1:
            nodeVectorPtr = &mSlaveNodesRightBoundary;
            std::cout << "mSlaveNodesRightBoundary " << std::endl;
            break;
        case 2:
            nodeVectorPtr = &mMasterNodesBottomBoundary;
            std::cout << "mMasterNodesBottomBoundary " << std::endl;
            break;
        default:
            nodeVectorPtr = &mSlaveNodesTopBoundary;
            std::cout << "mSlaveNodesTopBoundary " << std::endl;
            break;
        }
        for (unsigned int countNodes=0; countNodes<(*nodeVectorPtr).size(); countNodes++)
        {
            double coordinates[2];
            (*nodeVectorPtr)[countNodes]->GetCoordinates2D(coordinates);
            std::cout << "  " << mStructure->NodeGetId((*nodeVectorPtr)[countNodes]) << ": " << coordinates[0] << " " << coordinates[1] << std::endl;
        }
    }
}

//! @brief adds the constraint equations to the matrix
//! @param curConstraintEquation (is incremented during the function call)
//! @param rConstraintMatrix (the first row where a constraint equation is added is given by curConstraintEquation)
//! @param rRHS right hand side of the constraint equation
#define MIN_CONSTRAINT 1e-6
void NuTo::ConstraintDisplacementsPeriodic2D::AddToConstraintMatrix(int& curConstraintEquation,
        NuTo::SparseMatrixCSRGeneral<double>& rConstraintMatrix,
        NuTo::FullMatrix<double>& rRHS)const
{
    double LeftUpperCoordinates[2], LeftLowerCoordinates[2];
    mLeftUpperCorner->GetCoordinates2D(LeftUpperCoordinates);
    mLeftLowerCorner->GetCoordinates2D(LeftLowerCoordinates);

    //calculate length of specimen
    double length = LeftUpperCoordinates[1]-LeftLowerCoordinates[1];
    std::cout << "length " << length << std::endl;

    double crackShift;
    if (mAngle!=90)
        crackShift = length/tan(mAngle*PI/180.);
    else
        crackShift=0.;

    std::cout << "crackShift " << crackShift << std::endl;

    //only du linear interpolation between nodes on the boundary, this is not exact for quadrativ elements, but the effort is much reduced
    if (mAngle>=45 && mAngle<135)
    {
        double crackPosX = (length-crackShift)*0.5;
        //**************************************************************
        //add  constraints for all the slave nodes of the right boundary
        //**************************************************************
        assert(mMasterNodesLeftBoundary.size()>1);
        unsigned int nextMasterNodecount(1);

        NodeCoordinatesDisplacements2D* curMasterNodePtr(mMasterNodesLeftBoundary[0]);
        double coordinatesCurMaster[2];
        curMasterNodePtr->GetCoordinates2D(coordinatesCurMaster);

        NodeCoordinatesDisplacements2D* nextMasterNodePtr(mMasterNodesLeftBoundary[nextMasterNodecount]);;
        double coordinatesNextMaster[2];
        nextMasterNodePtr->GetCoordinates2D(coordinatesNextMaster);

        double deltaDisp[2];
        for (unsigned int countNode=0; countNode<mSlaveNodesRightBoundary.size(); countNode++)
        {
            NodeCoordinatesDisplacements2D* curSlaveNodePtr(mSlaveNodesRightBoundary[countNode]);
            double coordinatesSlave[2];
            mSlaveNodesRightBoundary[countNode]->GetCoordinates2D(coordinatesSlave);

            double coordinatesSlaveonMasterSide[2];
            coordinatesSlaveonMasterSide[0] = coordinatesSlave[0]-length;
            coordinatesSlaveonMasterSide[1] = coordinatesSlave[1];

            while (coordinatesNextMaster[1]<coordinatesSlaveonMasterSide[1] && nextMasterNodecount+1<mMasterNodesLeftBoundary.size())
            {
                 curMasterNodePtr = nextMasterNodePtr;
                 coordinatesCurMaster[0] = coordinatesNextMaster[0];
                 coordinatesCurMaster[1] = coordinatesNextMaster[1];
                 nextMasterNodecount++;

                 assert (nextMasterNodecount<mMasterNodesLeftBoundary.size());
                 nextMasterNodePtr = mMasterNodesLeftBoundary[nextMasterNodecount];
                 nextMasterNodePtr->GetCoordinates2D(coordinatesNextMaster);
            }
            //slave is between two master nodes or extrapolation with the first tow or last two nodes

            //calculate weighting function for each master node
            double w = this->CalculateWeightFunction(coordinatesCurMaster[1],coordinatesNextMaster[1],coordinatesSlaveonMasterSide[1]);

            //calculate delta displacement in x and y direction from the applied strain and the nodal position of slave node
            double dispSlave[2];
            this->CalculateDeltaDisp(coordinatesSlave,dispSlave);

            //calculate delta displacement in x and y direction from the applied strain and the nodal position of slave node
            double dispMaster[2];
            this->CalculateDeltaDisp(coordinatesSlaveonMasterSide,dispMaster);

            deltaDisp[0] = dispSlave[0] - dispMaster[0];
            deltaDisp[1] = dispSlave[1] - dispMaster[1];

            std::cout << "constraint equation " << curConstraintEquation
                    << ": node " << mStructure->NodeGetId(curSlaveNodePtr) << " + "
                    << w << " node " << mStructure->NodeGetId(curMasterNodePtr) << " + "
                    << 1-w << " node " << mStructure->NodeGetId(nextMasterNodePtr)
                    << " = (" << deltaDisp[0] << ", " << deltaDisp[1] << ")"
                    << std::endl;

            //constrain x direction
            rConstraintMatrix.AddEntry(curConstraintEquation,curSlaveNodePtr->GetDofDisplacement(0),1);
            if (fabs(w)>MIN_CONSTRAINT)
                rConstraintMatrix.AddEntry(curConstraintEquation,curMasterNodePtr->GetDofDisplacement(0),-w);
            if (fabs(w-1.)>MIN_CONSTRAINT)
                rConstraintMatrix.AddEntry(curConstraintEquation,nextMasterNodePtr->GetDofDisplacement(0),w-1.);
            rRHS(curConstraintEquation,0) = deltaDisp[0];
            curConstraintEquation++;

            //constrain y direction
            rConstraintMatrix.AddEntry(curConstraintEquation,curSlaveNodePtr->GetDofDisplacement(1),1);
            if (fabs(w)>MIN_CONSTRAINT)
                rConstraintMatrix.AddEntry(curConstraintEquation,curMasterNodePtr->GetDofDisplacement(1),-w);
            if (fabs(w-1.)>MIN_CONSTRAINT)
                rConstraintMatrix.AddEntry(curConstraintEquation,nextMasterNodePtr->GetDofDisplacement(1),w-1.);
            rRHS(curConstraintEquation,0) = deltaDisp[1];
            curConstraintEquation++;
        }

        //**************************************************************
        //add  constraints for all the slave nodes of the top boundary
        //**************************************************************
        assert(mMasterNodesBottomBoundary.size()>1);
        nextMasterNodecount = 1;

        curMasterNodePtr = mMasterNodesBottomBoundary[0];
        curMasterNodePtr->GetCoordinates2D(coordinatesCurMaster);
        coordinatesCurMaster[0]-=crackPosX;
        double deltaRHSCurMaster = 0;
        if (coordinatesCurMaster[0]<0)
        {
            coordinatesCurMaster[0]+=length;
            deltaRHSCurMaster += mStrain[0]*length;
        }

        nextMasterNodePtr = mMasterNodesBottomBoundary[nextMasterNodecount];
        nextMasterNodePtr->GetCoordinates2D(coordinatesNextMaster);
        coordinatesNextMaster[0]-=crackPosX;
        double deltaRHSNextMaster = 0;
        if (coordinatesNextMaster[0]<0)
        {
            coordinatesNextMaster[0]+=length;
            deltaRHSNextMaster += mStrain[0]*length;
        }


        for (unsigned int countNode=0; countNode<mSlaveNodesTopBoundary.size(); countNode++)
        {
            NodeCoordinatesDisplacements2D* curSlaveNodePtr(mSlaveNodesTopBoundary[countNode]);
            double coordinatesSlave[2];
            mSlaveNodesTopBoundary[countNode]->GetCoordinates2D(coordinatesSlave);

            double coordinatesSlaveonMasterSide[2];
            coordinatesSlaveonMasterSide[0] = coordinatesSlave[0]-crackShift;
            double deltaRHSSlave = -crackShift*mStrain[0]+length*mStrain[2];
            if (coordinatesSlaveonMasterSide[0]<0)
            {
                coordinatesSlaveonMasterSide[0]+=length;
                deltaRHSSlave += mStrain[0]*length;
            }
            coordinatesSlaveonMasterSide[1] = coordinatesSlave[1]-length;

            double coordinatesSlaveonMasterSideMinusCrackPosX[2];
            coordinatesSlaveonMasterSideMinusCrackPosX[0] = coordinatesSlaveonMasterSide[0]-crackPosX;
            if (coordinatesSlaveonMasterSideMinusCrackPosX[0]<0)
            {
                coordinatesSlaveonMasterSideMinusCrackPosX[0]+=length;
                deltaRHSSlave += mStrain[0]*length + mStrain[2]*length;
            }
            coordinatesSlaveonMasterSideMinusCrackPosX[1] = coordinatesSlaveonMasterSide[1];

            while (coordinatesNextMaster[0]<coordinatesSlaveonMasterSideMinusCrackPosX[0] && nextMasterNodecount+1<mMasterNodesBottomBoundary.size())
            {
                curMasterNodePtr = nextMasterNodePtr;
                coordinatesCurMaster[0] = coordinatesNextMaster[0];
                coordinatesCurMaster[1] = coordinatesNextMaster[1];
                deltaRHSCurMaster = deltaRHSNextMaster;
                nextMasterNodecount++;

                assert (nextMasterNodecount<mMasterNodesBottomBoundary.size());
                nextMasterNodePtr = mMasterNodesBottomBoundary[nextMasterNodecount];
                nextMasterNodePtr->GetCoordinates2D(coordinatesNextMaster);
                coordinatesNextMaster[0]-=crackPosX;
                deltaRHSNextMaster = 0.;
                if (coordinatesNextMaster[0]<0)
                {
                    coordinatesNextMaster[0]+=length;
                    deltaRHSNextMaster += mStrain[0]*length;
                }
            }

            //slave is between two master nodes or extrapolation with the first tow or last two nodes

            //calculate weighting function for each master node
            double w = this->CalculateWeightFunction(coordinatesCurMaster[0],coordinatesNextMaster[0],coordinatesSlaveonMasterSideMinusCrackPosX[0]);

            deltaDisp[0] = -deltaRHSSlave + w * deltaRHSCurMaster + (1.-w) * deltaRHSNextMaster ;
            deltaDisp[1] = length*(mStrain[1]+mStrain[2]);

            std::cout << "constraint equation " << curConstraintEquation
                    << ": node " << mStructure->NodeGetId(curSlaveNodePtr) << " + "
                    << -w << " node " << mStructure->NodeGetId(curMasterNodePtr) << " + "
                    << w-1 << " node " << mStructure->NodeGetId(nextMasterNodePtr)
                    << " = (" << deltaDisp[0] << ", " << deltaDisp[1] << ")"
                    << std::endl;

            //constrain x direction
            rConstraintMatrix.AddEntry(curConstraintEquation,curSlaveNodePtr->GetDofDisplacement(0),1);
            if (fabs(w)>MIN_CONSTRAINT)
                rConstraintMatrix.AddEntry(curConstraintEquation,curMasterNodePtr->GetDofDisplacement(0),-w);
            if (fabs(w-1.)>MIN_CONSTRAINT)
                rConstraintMatrix.AddEntry(curConstraintEquation,nextMasterNodePtr->GetDofDisplacement(0),w-1.);
            rRHS(curConstraintEquation,0) = deltaDisp[0];
            curConstraintEquation++;

            //constrain y direction
            rConstraintMatrix.AddEntry(curConstraintEquation,curSlaveNodePtr->GetDofDisplacement(1),1);
            if (fabs(w)>MIN_CONSTRAINT)
                rConstraintMatrix.AddEntry(curConstraintEquation,curMasterNodePtr->GetDofDisplacement(1),-w);
            if (fabs(w-1.)>MIN_CONSTRAINT)
                rConstraintMatrix.AddEntry(curConstraintEquation,nextMasterNodePtr->GetDofDisplacement(1),w-1.);
            rRHS(curConstraintEquation,0) = deltaDisp[1];
            curConstraintEquation++;
        }
    }
    else
    {
        throw MechanicsException("[NuTo::ConstraintDisplacementsPeriodic2D::SetBoundaryVectors] Not implemented for angles outside 0-90 degrees");
        if (mAngle>=135 && mAngle<225)
        {

        }
        else
        {
            if (mAngle>=225 && mAngle<315)
            {

            }
            else
            {
                assert((mAngle>=270 && mAngle<360) || (mAngle>=0 && mAngle<45));
            }
        }
    }
}

//! @brief returns the number of constraint equations
//! @return number of constraints
int NuTo::ConstraintDisplacementsPeriodic2D::GetNumConstraintEquations()const
{
    std::cout << "num constraints from periodic" << 2*(mSlaveNodesRightBoundary.size()+mSlaveNodesTopBoundary.size()) << std::endl;
    return 2*(mSlaveNodesRightBoundary.size()+mSlaveNodesTopBoundary.size());
}

//calculate weighting function for each master node
double NuTo::ConstraintDisplacementsPeriodic2D::CalculateWeightFunction(double rCoordinateCurMaster, double rCoordinateNextMaster, double rCoordinateSlave)const
{
    assert(rCoordinateNextMaster!=rCoordinateCurMaster);
    return 1.-(rCoordinateSlave-rCoordinateCurMaster)/(rCoordinateNextMaster-rCoordinateCurMaster);
}

//calculate delta displacement in x and y direction from the applied strain and the nodal position
void NuTo::ConstraintDisplacementsPeriodic2D::CalculateDeltaDisp(double rCoordinates[2], double rDeltaDisp[2])const
{
    rDeltaDisp[0] = mStrain[0]*rCoordinates[0] + mStrain[2]*rCoordinates[1];
    rDeltaDisp[1] = mStrain[1]*rCoordinates[1] + mStrain[2]*rCoordinates[0];
}

#ifdef ENABLE_SERIALIZATION
// serialize
template void NuTo::ConstraintDisplacementsPeriodic2D::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintDisplacementsPeriodic2D::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintDisplacementsPeriodic2D::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintDisplacementsPeriodic2D::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintDisplacementsPeriodic2D::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintDisplacementsPeriodic2D::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::ConstraintDisplacementsPeriodic2D::serialize(Archive & ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstraintBase)
    & BOOST_SERIALIZATION_NVP(mAngle)
    & BOOST_SERIALIZATION_NVP(mStrain);
}
#endif // ENABLE_SERIALIZATION
