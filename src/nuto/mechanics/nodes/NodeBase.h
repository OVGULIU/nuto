// $Id$
#ifndef NODEBASE_H
#define NODEBASE_H

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/array.hpp>
#else
#include <boost/array.hpp>
#endif  // ENABLE_SERIALIZATION

#include <vector>

#include "nuto/math/FullMatrix_Def.h"
#include "nuto/math/FullVector_Def.h"
#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/nodes/NodeEnum.h"

#ifdef ENABLE_VISUALIZE
#include "nuto/visualize/VisualizeBase.h"
#include "nuto/visualize/VisualizeComponentBase.h"
#include "nuto/visualize/VisualizeUnstructuredGrid.h"
#include <boost/ptr_container/ptr_list.hpp>
#endif // ENABLE_VISUALIZE

namespace NuTo
{
class NodeDisplacementsMultiscale2D;
//! @author Jörg F. Unger, ISM
//! @date October 2009
//! @brief ... standard abstract class for all nodes
class NodeBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief constructor
    NodeBase();

    //! @brief destructor
    virtual ~NodeBase(){};

    //! @brief assignment operator
    void operator=(NodeBase const& rOther)
    {
    }

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION

    //! @brief sets the global dofs
    //! @param rDOF current maximum DOF, this variable is increased within the routine
    virtual void SetGlobalDofs(int& rDOF);

    //! @brief write dof values to the node (based on global dof number)
    //! @param rTimeDerivative ... time derivative (e.g. 0 disp, 1 vel, 2 acc)
    //! @param rActiveDofValues ... active dof values
    //! @param rDependentDofValues ... dependent dof values
    virtual void SetGlobalDofValues(int rTimeDerivative, const FullVector<double,Eigen::Dynamic>& rActiveDofValues, const FullVector<double,Eigen::Dynamic>& rDependentDofValues);

    //! @brief extract dof values from the node (based on global dof number)
    //! @param rTimeDerivative ... time derivative (e.g. 0 disp, 1 vel, 2 acc)
    //! @param rActiveDofValues ... active dof values
    //! @param rDependentDofValues ... dependent dof values
    virtual void GetGlobalDofValues(int rTimeDerivative, FullVector<double,Eigen::Dynamic>& rActiveDofValues, FullVector<double,Eigen::Dynamic>& rDependentDofValues) const;

    //! @brief extract all dof numbers from the node (based on global dof number)
    //virtual int* GetGlobalDofs();

    //! @brief renumber the global dofs according to predefined ordering
    //! @param rMappingInitialToNewOrdering ... mapping from initial ordering to the new ordering
    virtual void RenumberGlobalDofs(std::vector<int>& rMappingInitialToNewOrdering);

    //! @brief returns the number of coordinates of the node
    //! @return number of coordinates
    virtual int GetNumCoordinates()const;

    //! @brief returns the coordinates of the node
    //! @return coordinates
    virtual void GetCoordinates1D(double rCoordinates[1])const;

    //! @brief set the coordinates
    //! @param rCoordinates  given coordinates
    virtual void SetCoordinates1D(const double rCoordinates[1]);

    //! @brief returns the coordinates of the node
    //! @return coordinates
    virtual void GetCoordinates2D(double rCoordinates[2])const;

    //! @brief set the coordinates
    //! @param rCoordinates  given coordinates
    virtual void SetCoordinates2D(const double rCoordinates[2]);

    //! @brief returns the coordinates of the node
    //! @return coordinates
    virtual void GetCoordinates3D(double rCoordinates[3])const;

    //! @brief set the coordinates
    //! @param rCoordinates  given coordinates
    virtual void SetCoordinates3D(const double rCoordinates[3]);

    //! @brief returns the number of coordinates of the node
    //! @return coordinates
    virtual double GetCoordinate(short rIndex)const;

    //! @brief returns the number of time derivatives stored at the node
    //! @return number of derivatives
    virtual int GetNumTimeDerivatives()const;

    //! @brief returns the number of displacements of the node
    //! @return number of displacements
    virtual int GetNumDisplacements()const;

    //! @brief gives the global DOF of a displacement component
    //! @param rComponent component
    //! @return global DOF
    virtual int GetDofDisplacement(int rComponent)const;

    //! @brief returns the displacements of the node
    //! @return displacement
    virtual void GetDisplacements1D(double rCoordinates[1])const;

    //! @brief returns the displacements of the node
    //! @param rTimeDerivative time derivative
    //! @return displacement
    virtual void GetDisplacements1D(int rTimeDerivative, double rDisplacements[1])const;

    //! @brief set the displacements
    //! @param rDisplacements  given displacements
    virtual void SetDisplacements1D(const double rDisplacements[1]);

    //! @brief set the displacements
    //! @param rTimeDerivative time derivative
    //! @param rDisplacements  given displacements
    virtual void SetDisplacements1D(int rTimeDerivative, const double rDisplacements[1]);

    //! @brief returns the displacements of the node
    //! @return displacement
    virtual void GetDisplacements2D(double rDisplacements[2])const;

    //! @brief returns the displacements of the node
    //! @param rTimeDerivative time derivative
    //! @return displacement
    virtual void GetDisplacements2D(int rTimeDerivative, double rDisplacements[2])const;

    //! @brief set the displacements
    //! @param rDisplacements  given displacements
    virtual void SetDisplacements2D(const double rDisplacements[2]);

    //! @brief set the displacements
    //! @param rTimeDerivative time derivative
    //! @param rDisplacements  given displacements
    virtual void SetDisplacements2D(int rTimeDerivative, const double rDisplacements[2]);

    //! @brief returns the displacements of the node
    //! @return displacement
    virtual void GetDisplacements3D(double rDisplacements[3])const;

    //! @brief returns the displacements of the node
    //! @param rTimeDerivative time derivative
    //! @return displacement
    virtual void GetDisplacements3D(int rTimeDerivative, double rDisplacements[3])const;

    //! @brief set the displacements
    //! @param rDisplacements  given displacements
    virtual void SetDisplacements3D(const double rDisplacements[3]);

    //! @brief set the displacements
    //! @param rTimeDerivative time derivative
    //! @param rDisplacements  given displacements
    virtual void SetDisplacements3D(int rTimeDerivative, const double rDisplacements[3]);

    //! @brief returns the displacements of the node
    //! @return displacement
    virtual double GetDisplacement(short rIndex)const;

    //! @brief returns the number of Rotations of the node
    //! @return number of Rotations
    virtual int GetNumRotations()const;

    //! @brief gives the global DOF of a Rotation component
    //! @param rComponent component
    //! @return global DOF
    virtual int GetDofRotation(int rComponent)const;

    //! @brief returns the Rotations of the node
    //! @return Rotation
    virtual void GetRotations2D(double rRotations[1])const;

    //! @brief returns the Rotations of the node
    //! @param rTimeDerivative time derivative
    //! @return Rotation
    virtual void GetRotations2D(int rTimeDerivative, double rRotations[1])const;

    //! @brief set the Rotations
    //! @param rRotations  given Rotations
    virtual void SetRotations2D(const double rRotations[1]);

    //! @brief set the Rotations
    //! @param rTimeDerivative time derivative
    //! @param rRotations  given Rotations
    virtual void SetRotations2D(int rTimeDerivative, const double rRotations[1]);

    //! @brief returns the Rotations of the node
    //! @return Rotation
    virtual void GetRotations3D(double rRotations[3])const;

    //! @brief returns the Rotations of the node
    //! @param rTimeDerivative time derivative
    //! @return Rotation
    virtual void GetRotations3D(int rTimeDerivative, double rRotations[1])const;

    //! @brief set the Rotations
    //! @param rRotations  given Rotations
    virtual void SetRotations3D(const double rRotations[3]);

    //! @brief set the Rotations
    //! @param rTimeDerivative time derivative
    //! @param rRotations  given Rotations
    virtual void SetRotations3D(int rTimeDerivative, const double rRotations[3]);

    //! @brief returns the Rotations of the node
    //! @return Rotation
    virtual double GetRotation(short rIndex)const;

    //! @brief returns the number of temperatures of the node
    //! @return number of temperatures
    virtual int GetNumTemperatures()const;

    //! @brief returns the temperature of the node
    //! @return temperature
    virtual double GetTemperature()const;

    //! @brief returns the temperature of the node
    //! @param rTimeDerivative time derivative
    //! @return temperature
    virtual double GetTemperature(int rTimeDerivative)const;

    //! @brief set the temperature of the node
    //! @param rTemperature  given temperature
    virtual void SetTemperature(double rTemperature);

    //! @brief set the temperature of the node
    //! @param rTimeDerivative time derivative
    //! @param rTemperature  given temperature
    virtual void SetTemperature(int rTimeDerivative, double rTemperature);

    //! @brief gives the global DOF of a temperature component
    //! @return global DOF
    virtual int GetDofTemperature()const;

    //! @brief returns the number of nonlocal equivalent plastic strain dofs of the node
    //! @return number of Damages
    virtual int GetNumNonlocalEqPlasticStrain()const;

    //! @brief returns the Damage of the node
    //! @return Damage
    virtual void GetNonlocalEqPlasticStrain(double* rNonlocalEqPlasticStrain)const;

    //! @brief returns the Damage of the node
    //! @param rTimeDerivative time derivative
    //! @return Damage
    virtual void GetNonlocalEqPlasticStrain(int rTimeDerivative, double* rNonlocalEqPlasticStrain)const;

    //! @brief set the Damage of the node
    //! @param rDamage  given Damage
    virtual void SetNonlocalEqPlasticStrain(const double* rNonlocalEqPlasticStrain);

    //! @brief set the Damage of the node
    //! @param rTimeDerivative time derivative
    //! @param rDamage  given Damage
    virtual void SetNonlocalEqPlasticStrain(int rTimeDerivative, const double* rNonlocalEqPlasticStrain);

    //! @brief gives the global DOF of a Damage component
    //! @param rComponent component
    //! @return global DOF
    virtual int GetDofNonlocalEqPlasticStrain(int rComponent)const;

    //! @brief returns the number of nonlocal equivalent plastic strain dofs of the node
    //! @return number of Damages
    virtual int GetNumNonlocalTotalStrain()const;

    //! @brief returns the Damage of the node
    //! @return Damage
    virtual void GetNonlocalTotalStrain1D(double* rNonlocalTotalStrain)const;

    //! @brief returns the Damage of the node
    //! @return Damage
    virtual void GetNonlocalTotalStrain2D(double* rNonlocalTotalStrain)const;

    //! @brief returns the Damage of the node
    //! @return Damage
    virtual void GetNonlocalTotalStrain3D(double* rNonlocalTotalStrain)const;

    //! @brief returns the Damage of the node
    //! @param rTimeDerivative time derivative
    //! @return Damage
    virtual void GetNonlocalTotalStrain1D(int rTimeDerivative, double* rNonlocalTotalStrain)const;

    //! @brief returns the Damage of the node
    //! @param rTimeDerivative time derivative
    //! @return Damage
    virtual void GetNonlocalTotalStrain2D(int rTimeDerivative, double* rNonlocalTotalStrain)const;

    //! @brief returns the Damage of the node
    //! @param rTimeDerivative time derivative
    //! @return Damage
    virtual void GetNonlocalTotalStrain3D(int rTimeDerivative, double* rNonlocalTotalStrain)const;

    //! @brief set the Damage of the node
    //! @param rDamage  given Damage
    virtual void SetNonlocalTotalStrain1D(const double* rNonlocalTotalStrain);

    //! @brief set the Damage of the node
    //! @param rDamage  given Damage
    virtual void SetNonlocalTotalStrain2D(const double* rNonlocalTotalStrain);

    //! @brief set the Damage of the node
    //! @param rDamage  given Damage
    virtual void SetNonlocalTotalStrain3D(const double* rNonlocalTotalStrain);

    //! @brief set the Damage of the node
    //! @param rTimeDerivative time derivative
    //! @param rDamage  given Damage
    virtual void SetNonlocalTotalStrain1D(int rTimeDerivative, const double* rNonlocalTotalStrain);

    //! @brief set the Damage of the node
    //! @param rTimeDerivative time derivative
    //! @param rDamage  given Damage
    virtual void SetNonlocalTotalStrain2D(int rTimeDerivative, const double* rNonlocalTotalStrain);

    //! @brief set the Damage of the node
    //! @param rTimeDerivative time derivative
    //! @param rDamage  given Damage
    virtual void SetNonlocalTotalStrain3D(int rTimeDerivative, const double* rNonlocalTotalStrain);

    //! @brief returns the nonlocal total strain component of the node
    //! @return strain component (rTimeDerivative=0)
    virtual double GetNonlocalTotalStrain(short rIndex)const;

    //! @brief gives the global DOF of a Damage component
    //! @param rComponent component
    //! @return global DOF
    virtual int GetDofNonlocalTotalStrain(int rComponent)const;

    //! @brief returns the number of temperatures of the node
    //! @return number of temperatures
    virtual int GetNumNonlocalEqStrain()const;

    //! @brief returns the nonlocal eq. strain of the node
    //! @return nonlocal eq. strain
    virtual double GetNonlocalEqStrain()const;

    //! @brief returns the nonlocal eq. strain of the node
    //! @param rTimeDerivative time derivative
    //! @return nonlocal eq. strain
    virtual double GetNonlocalEqStrain(int rTimeDerivative)const;

    //! @brief set the nonlocal eq. strain of the node
    //! @param rNonlocalEqStrain  given nonlocal eq. strain
    virtual void SetNonlocalEqStrain(double rNonlocalEqStrain);

    //! @brief set the temperature of the node
    //! @param rTimeDerivative time derivative
    //! @param rNonlocalEqStrain given nonlocal eq. strain
    virtual void SetNonlocalEqStrain(int rTimeDerivative, double rNonlocalEqStrain);

    //! @brief gives the global DOF the nonlocal eq. strain
    //! @return global DOF
    virtual int GetDofNonlocalEqStrain()const;

    // Moisture Transport --- Begin

    // WaterPhaseFraction, int TNumRelativeHumidity

    //! @brief returns the number of water phase fraction components of the node
    //! @return number of water phase fraction components
    virtual int GetNumWaterPhaseFraction()const;

    //! @brief returns the water phase fraction of the node
    //! @return water phase fraction
    virtual double GetWaterPhaseFraction()const;

    //! @brief returns the water phase fraction of the node
    //! @param rTimeDerivative time derivative
    //! @return water phase fraction
    virtual double GetWaterPhaseFraction(int rTimeDerivative)const;

    //! @brief set the water phase fraction of the node
    //! @param rTemperature  given temperature
    virtual void SetWaterPhaseFraction(double rWaterPhaseFraction);

    //! @brief set the water phase fraction of the node
    //! @param rTimeDerivative time derivative
    //! @param rTemperature  given temperature
    virtual void SetWaterPhaseFraction(int rTimeDerivative, double rWaterPhaseFraction);

    //! @brief gives the global DOF of a water phase fraction component
    //! @param rComponent component
    //! @return global DOF
    virtual int GetDofWaterPhaseFraction()const;



    // Moisture Transport --- End


    //! @brief returns the type of node as a string (all the data stored at the node)
    //! @return string
    virtual std::string GetNodeTypeStr()const=0;

    //! @brief returns the type of node as an enum (all the data stored at the node)
    //! @return enum
    virtual Node::eNodeType GetNodeType()const
    {
    	throw MechanicsException("[NodeBase::GetNodeType] routine is removed");
    }

#ifdef ENABLE_VISUALIZE
    virtual void Visualize(VisualizeUnstructuredGrid& rVisualize, const boost::ptr_list<NuTo::VisualizeComponentBase>& rWhat) const;

#endif // ENABLE_VISUALIZE

    //! @brief clones (copies) the node with all its data, it's supposed to be a new node, so be careful with ptr
    virtual NodeBase* Clone()const=0;

protected:
    //the base class of the nodes must not contain any data

};

class less_XCoordinate2D : public std::binary_function<NodeBase*, NodeBase* , bool>
{
public:

    //! @brief sorts the nodes in increasing x-direction
    less_XCoordinate2D()
    {
    }

    bool operator()(NodeBase* nodePtr1, NodeBase* nodePtr2)
    {
        double coord1[2], coord2[2];
        nodePtr1->GetCoordinates2D(coord1);
        nodePtr2->GetCoordinates2D(coord2);
        return coord1[0] < coord2[0];
    }
};

class greater_XCoordinate2D : public std::binary_function<NodeBase*, NodeBase* , bool>
{
public:
    greater_XCoordinate2D()
    {
    }

    bool operator()(NodeBase* nodePtr1, NodeBase* nodePtr2)
    {
        double coord1[2], coord2[2];
        nodePtr1->GetCoordinates2D(coord1);
        nodePtr2->GetCoordinates2D(coord2);
        return coord1[0] > coord2[0];
    }
};

class less_YCoordinate2D : public std::binary_function<NodeBase*, NodeBase* , bool>
{
public:
    less_YCoordinate2D()
    {
    }

    bool operator()(NodeBase* nodePtr1, NodeBase* nodePtr2)
    {
        double coord1[2], coord2[2];
        nodePtr1->GetCoordinates2D(coord1);
        nodePtr2->GetCoordinates2D(coord2);
        return coord1[1] < coord2[1];
    }
};

class greater_YCoordinate2D : public std::binary_function<NodeBase*, NodeBase* , bool>
{
public:
    greater_YCoordinate2D()
    {
    }

    bool operator()(NodeBase* nodePtr1, NodeBase* nodePtr2)
    {
        double coord1[2], coord2[2];
        nodePtr1->GetCoordinates2D(coord1);
        nodePtr2->GetCoordinates2D(coord2);
        return coord1[1] > coord2[1];
    }
};

}//namespace NuTo
#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(NuTo::NodeBase)
#endif // ENABLE_SERIALIZATION
#endif //NODEBASE_H

