// $Id: ConstraintNodeGroupDisplacements2D.h 265 2010-06-08 08:47:00Z arnold2 $

#ifndef CONSTRAINTDISPLACEMENTS2PERIODIC2D_H
#define CONSTRAINTDISPLACEMENTS2PERIODIC2D_H

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#endif  // ENABLE_SERIALIZATION

#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/constraints/ConstraintNodeGroup.h"

namespace NuTo
{
class NodeCoordinatesDisplacements2D;
//! @author Joerg F. Unger
//! @date August 2010
//! @brief ... class for all displacement constraints periodic boundary conditions in 2D
class ConstraintDisplacementsPeriodic2D : public ConstraintBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief constructor
    //! @param rDirection ... direction of the applied constraint
    //! @param rValue ... direction of the applied constraint
    ConstraintDisplacementsPeriodic2D(const StructureBase* rStructure, double rAngle, NuTo::FullMatrix<double> rStrain,
            const Group<NodeBase>* rGroupTop,const Group<NodeBase>* rGroupBottom,
            const Group<NodeBase>* rGroupLeft, const Group<NodeBase>* rGroupRight);

    //!@brief sets/modifies angle of the boundary condition
    //!@param rAngle angle in deg
    void SetAngle(double rAngle);

    //!@brief sets/modifies the average strain applied to the boundary
    //!@param rAngle angle in deg
    void SetStrain(const NuTo::FullMatrix<double>& rStrain);

    //!@brief calculate the border vectors in counterclockwise direction
    void SetBoundaryVectors();

    //! @brief returns the number of constraint equations
    //! @return number of constraints
    int GetNumConstraintEquations()const;

    //! @brief adds the constraint equations to the matrix
    //! @param curConstraintEquation (is incremented during the function call)
    //! @param rConstraintMatrix (the first row where a constraint equation is added is given by curConstraintEquation)
    //! @param rRHS right hand side of the constraint equation
    void AddToConstraintMatrix(int& curConstraintEquation,
                               NuTo::SparseMatrixCSRGeneral<double>& rConstraintMatrix,
                               NuTo::FullMatrix<double>& rRHS)const;

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION

protected:
    //! @brief calculate weighting function for first master node (linear interpolation between master nodes
    //! @param rCoordinateCurMaster
    double CalculateWeightFunction(double rCoordinateCurMaster, double rCoordinateNextMaster, double rCoordinateSlave)const;

    //! @brief calculate delta displacement in x and y direction from the applied strain and the nodal position

    void CalculateDeltaDisp(double rCoordinates[2], double rDeltaDisp[2])const;

    //! @brief prescribed angle of the periodoc boundary conditions
    double mAngle;

    //! @brief average strain applied to the boundaries (epsilon_xx, epsilon_yy, gamma_xy)
    double mStrain[3];

    //! @brief boundary groups (upper, lower, left and right), corner nodes are supposed to be in two groups
    const Group<NodeBase>* mGroupTop;
    const Group<NodeBase>* mGroupBottom;
    const Group<NodeBase>* mGroupLeft;
    const Group<NodeBase>* mGroupRight;

    const NodeBase* mLeftUpperCorner;
    const NodeBase* mLeftLowerCorner;
    const NodeBase* mRightUpperCorner;
    const NodeBase* mRightLowerCorner;

    std::vector<NodeCoordinatesDisplacements2D*> mSlaveNodesRightBoundary;
    std::vector<NodeCoordinatesDisplacements2D*> mSlaveNodesTopBoundary;
    std::vector<NodeCoordinatesDisplacements2D*> mMasterNodesLeftBoundary;
    std::vector<NodeCoordinatesDisplacements2D*> mMasterNodesBottomBoundary;

    const StructureBase* mStructure;
};
}//namespace NuTo
#endif //CONSTRAINTDISPLACEMENTS2PERIODIC2D_H