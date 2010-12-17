// $Id: ConstraintLagrangeNodeDisplacements1D.cpp -1   $

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif  // ENABLE_SERIALIZATION

#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/nodes/NodeDisplacements1D.h"
#include "nuto/mechanics/groups/Group.h"
#include "nuto/mechanics/constraints/ConstraintLagrangeNodeGroupDisplacements1D.h"
#include "nuto/math/FullMatrix.h"
#include "nuto/math/SparseMatrixCSRVector2Symmetric.h"

NuTo::ConstraintLagrangeNodeGroupDisplacements1D::ConstraintLagrangeNodeGroupDisplacements1D(const Group<NodeBase>* rGroup, const NuTo::FullMatrix<double>& rDirection, NuTo::Constraint::eEquationSign rEquationSign, double rRHS) :
        ConstraintNodeGroup(rGroup), ConstraintLagrange(rEquationSign)
{
    if (rDirection.GetNumColumns()!=1 || rDirection.GetNumRows()!=1)
        throw MechanicsException("[NuTo::ConstraintLagrangeNodeGroupDisplacements1D::ConstraintLagrangeNodeGroupDisplacements1D] Dimension of the direction matrix must be equal to the dimension of the structure.");

    mRHS = rRHS;

    mLagrangeValue.resize(mGroup->GetNumMembers(),0);
    mLagrangeDOF.resize(mGroup->GetNumMembers());
}

//! @brief returns the number of constraint equations
//! @return number of constraints
int NuTo::ConstraintLagrangeNodeGroupDisplacements1D::GetNumLagrangeMultipliers()const
{
    return mLagrangeValue.size();
}

//! @brief returns the Lagrange Multiplier
//! first col Lagrange, second column slack variables
void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::GetLagrangeGetMultiplier(FullMatrix<double>& rLagrangeMultiplier)const
{
    rLagrangeMultiplier.Resize(mGroup->GetNumMembers(),1);
    for (unsigned int count=0; count<mLagrangeValue.size(); count++)
        rLagrangeMultiplier(count,0) = mLagrangeValue[count];
}

//!@brief sets/modifies the right hand side of the constraint equations
//!@param rRHS new right hand side
void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::SetRHS(double rRHS)
{
    mRHS = rRHS;
}

//! @brief sets the global dofs
//! @param rDOF current maximum DOF, this variable is increased within the routine
void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::SetGlobalDofs(int& rDOF)
{
    assert((int)mLagrangeDOF.size()==mGroup->GetNumMembers());

    for (unsigned int count=0; count<mLagrangeDOF.size(); count++)
    {
        mLagrangeDOF[count] = rDOF++;
    }
}

//! @brief write dof values to constraints (based on global dof number)
//! @param rActiveDofValues ... active dof values
//! @param rDependentDofValues ... dependent dof values
void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::SetGlobalDofValues(const FullMatrix<double>& rActiveDofValues, const FullMatrix<double>& rDependentDofValues)
{
    assert(rActiveDofValues.GetNumColumns() == 1);
    assert(rDependentDofValues.GetNumColumns() == 1);

    for (unsigned int count=0; count<mLagrangeDOF.size(); count++)
    {
        int dof = this->mLagrangeDOF[count];
        double value;
        if (dof >= rActiveDofValues.GetNumRows())
        {
            dof -= rActiveDofValues.GetNumRows();
            assert(dof < rDependentDofValues.GetNumRows());
            value = rDependentDofValues(dof,0);
        }
        else
        {
            value = rActiveDofValues(dof,0);
        }
        this->mLagrangeValue[count] = value;
    }
}

//! @brief extract dof values from the constraints (based on global dof number)
//! @param rActiveDofValues ... active dof values
//! @param rDependentDofValues ... dependent dof values
void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::GetGlobalDofValues(FullMatrix<double>& rActiveDofValues, FullMatrix<double>& rDependentDofValues) const
{
    assert(rActiveDofValues.GetNumColumns() == 1);
    assert(rDependentDofValues.GetNumColumns() == 1);

    for (unsigned int count=0; count<mLagrangeDOF.size(); count++)
    {
        int dof = this->mLagrangeDOF[count];
        double value = this->mLagrangeValue[count];
        if (dof >= rActiveDofValues.GetNumRows())
        {
            dof -= rActiveDofValues.GetNumRows();
            assert(dof < rDependentDofValues.GetNumRows());
            rDependentDofValues(dof,0) = value;
        }
        else
        {
            rActiveDofValues(dof,0) = value;
        }
    }
}

//! @brief renumber the global dofs according to predefined ordering
//! @param rMappingInitialToNewOrdering ... mapping from initial ordering to the new ordering
void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::RenumberGlobalDofs(const std::vector<int>& rMappingInitialToNewOrdering)
{
    for (unsigned int count=0; count<mLagrangeDOF.size(); count++)
    {
        mLagrangeDOF[count] = rMappingInitialToNewOrdering[mLagrangeDOF[count]];
    }
}

//! @brief cast to Lagrange constraint - the corresponding dofs are eliminated in the global system
NuTo::ConstraintLagrange* NuTo::ConstraintLagrangeNodeGroupDisplacements1D::AsConstraintLagrange()
{
    return this;
}

//! @brief cast to Lagrange constraint - the corresponding dofs are eliminated in the global system
const NuTo::ConstraintLagrange* NuTo::ConstraintLagrangeNodeGroupDisplacements1D::AsConstraintLagrange()const
{
    return this;
}

//! @brief calculates the coefficient matrix for the 0-th derivative in the differential equation
//! for a mechanical problem, this corresponds to the stiffness matrix (Hessian) of the total potential
//! @param rResult ... coefficient matrix
//! @param rGlobalDofs ... row and column numbers in global system
void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::CalculateCoefficientMatrix_0(NuTo::SparseMatrixCSRVector2Symmetric<double>& rResult,
        std::vector<int>& rGlobalDofs)const
{
    //Lagrange mult + disp Dofs of all nodes
    int dof(2*mLagrangeDOF.size());
    rResult.Resize(dof,dof);
    rGlobalDofs.resize(dof,1);
    int curNodeEntry(0);
    int theNode(0);
    for (Group<NodeBase>::iterator itNode=mGroup->begin(); itNode!=mGroup->end();itNode++, curNodeEntry+=2, theNode++)
    {
        double disp[1];
        int dofx((*itNode)->GetDofDisplacement(0));
        (*itNode)->GetDisplacements1D(disp);

        rGlobalDofs[curNodeEntry] = mLagrangeDOF[theNode];
        rGlobalDofs[curNodeEntry+1] = dofx;

        switch(mEquationSign)
        {
        case NuTo::Constraint::EQUAL:
            //derivative with respect to ux and lambda
             rResult.AddEntry(curNodeEntry,curNodeEntry+1,1.);
            //derivative with respect to ux^2
            rResult.AddEntry(curNodeEntry+1,curNodeEntry+1,mPenalty);
        break;
        case NuTo::Constraint::SMALLER:
            if (disp[0]-mRHS>-mLagrangeValue[theNode]/mPenalty)
            {
                //derivative with respect to ux and lambda
                 rResult.AddEntry(curNodeEntry,curNodeEntry+1,1.);
                //derivative with respect to ux^2
                rResult.AddEntry(curNodeEntry+1,curNodeEntry+1,mPenalty);
            }
            else
            {
                //derivative with respect to lambda^2
                rResult.AddEntry(curNodeEntry,curNodeEntry,-1./mPenalty);
            }
        break;
        case NuTo::Constraint::GREATER:
            if (mRHS-disp[0]>-mLagrangeValue[theNode]/mPenalty)
            {
                //derivative with respect to ux and lambda
                 rResult.AddEntry(curNodeEntry,curNodeEntry+1,-1.);
                //derivative with respect to ux^2
                rResult.AddEntry(curNodeEntry+1,curNodeEntry+1,mPenalty);
            }
            else
            {
                //derivative with respect to lambda^2
                rResult.AddEntry(curNodeEntry,curNodeEntry,-1./mPenalty);
            }
        break;
        default:
            throw MechanicsException("[NuTo::ConstraintLagrangeNodeGroupDisplacements1D::CalculateCoefficientMatrix_0] equation sign should be either EQUAL, SMALLER or GREAER.");
        }
    }
}

//! @brief calculates the gradient of the internal potential
//! for a mechanical problem, this corresponds to the internal force vector
void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::CalculateGradientInternalPotential(NuTo::FullMatrix<double>& rResult,
        std::vector<int>& rGlobalDofs)const
{
    int dof(2*mLagrangeDOF.size());
    rResult.Resize(dof,1);
    rGlobalDofs.resize(dof);
    int curNodeEntry(0);
    int theNode(0);
    for (Group<NodeBase>::iterator itNode=mGroup->begin(); itNode!=mGroup->end();itNode++, curNodeEntry+=2, theNode++)
    {
        double disp[1];
        int dofx((*itNode)->GetDofDisplacement(0));
        (*itNode)->GetDisplacements1D(disp);

        rGlobalDofs[curNodeEntry] = mLagrangeDOF[theNode];
        rGlobalDofs[curNodeEntry+1] = dofx;

        switch(mEquationSign)
        {
        case NuTo::Constraint::EQUAL:
            //derivative with respect to lambda
            rResult(curNodeEntry,0)=disp[0]-mRHS;
            //derivative with respect to displacement
            rResult(curNodeEntry+1,0)=mLagrangeValue[theNode]+mPenalty*(disp[0]-mRHS);
        break;
        case NuTo::Constraint::SMALLER:
            if (disp[0]-mRHS>-mLagrangeValue[theNode]/mPenalty)
            {
                //derivative with respect to lambda
                rResult(curNodeEntry,0)=disp[0]-mRHS;
                //derivative with respect to displacement
                rResult(curNodeEntry+1,0)=mLagrangeValue[theNode]+mPenalty*(disp[0]-mRHS);
            }
            else
            {
                //derivative with respect to lambda
                rResult(curNodeEntry,0)=-mLagrangeValue[theNode]/mPenalty;
            }
        break;
        case NuTo::Constraint::GREATER:
            if (mRHS-disp[0]>-mLagrangeValue[theNode]/mPenalty)
            {
                //derivative with respect to lambda
                rResult(curNodeEntry,0)=mRHS-disp[0];
                //derivative with respect to displacement
                rResult(curNodeEntry+1,0)=-mLagrangeValue[theNode]-mPenalty*(mRHS-disp[0]);
            }
            else
            {
                //derivative with respect to lambda
                rResult(curNodeEntry,0)=-mLagrangeValue[theNode]/mPenalty;
            }
        break;
        default:
            throw MechanicsException("[NuTo::ConstraintLagrangeNodeGroupDisplacements1D::CalculateCoefficientMatrix_0] equation sign should be either EQUAL, SMALLER or GREAER.");
        }
    }
}

#ifdef ENABLE_SERIALIZATION
// serializes the class
template void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::ConstraintLagrangeNodeGroupDisplacements1D::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize ConstraintLagrangeNodeGroupDisplacements1D" << std::endl;
#endif
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstraintNodeGroup)
       & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstraintLagrange)
       & BOOST_SERIALIZATION_NVP(mRHS)
       & BOOST_SERIALIZATION_NVP(mLagrangeValue)
       & BOOST_SERIALIZATION_NVP(mLagrangeDOF);
#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize ConstraintLagrangeNodeGroupDisplacements1D" << std::endl;
#endif
}
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::ConstraintLagrangeNodeGroupDisplacements1D)
#endif // ENABLE_SERIALIZATION