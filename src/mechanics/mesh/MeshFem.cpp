#include "mechanics/mesh/MeshFem.h"

#include <sstream>
#include "base/Exception.h"

using namespace NuTo;

InterpolationSimple& MeshFem::CreateInterpolation(const InterpolationSimple& interpolation)
{
    mInterpolations.push_back(interpolation.Clone().release());
    return *mInterpolations.rbegin();
}

NodeSimple& MeshFem::NodeAtCoordinate(Eigen::VectorXd coords, DofType dofType, double tol /* = 1.e-10 */)
{
    for (auto& element : this->Elements)
    {
        auto& dofElement = element.DofElement(dofType);
        const auto& dofInterpolation = dofElement.Interpolation();
        for (int iNode = 0; iNode < dofInterpolation.GetNumNodes(); ++iNode)
        {
            NaturalCoords dofNodeCoords = dofInterpolation.GetLocalCoords(iNode);
            Eigen::VectorXd globalNodeCoords = Interpolate(element.CoordinateElement(), dofNodeCoords);
            if ((globalNodeCoords - coords).isMuchSmallerThan(tol))
                return dofElement.GetNode(iNode);
        }
    }
    std::stringstream coordsString;
    coordsString << coords.transpose();
    throw NuTo::Exception(__PRETTY_FUNCTION__,
                          "There is no node for dof type " + dofType.GetName() + " at " + coordsString.str());
}

NodeSimple& MeshFem::NodeAtCoordinate(Eigen::VectorXd coords, double tol /* = 1.e-10 */)
{
    for (auto& element : this->Elements)
    {
        auto& coordinateElement = element.CoordinateElement();
        for (int iNode = 0; iNode < coordinateElement.Interpolation().GetNumNodes(); ++iNode)
        {
            Eigen::VectorXd globalNodeCoords = coordinateElement.GetNode(iNode).GetValues();
            if ((globalNodeCoords - coords).isMuchSmallerThan(tol))
                return coordinateElement.GetNode(iNode);
        }
    }
    std::stringstream coordsString;
    coordsString << coords.transpose();
    throw NuTo::Exception(__PRETTY_FUNCTION__, "There is no coordinate node at " + coordsString.str());
}

Groups::Group<NodeSimple> MeshFem::NodesAtAxis(eDirection direction, double axisOffset /* = 0.*/,
                                               double tol /* = 1.e-10 */)
{
    Groups::Group<NodeSimple> group;
    const int directionComponent = ToComponentIndex(direction);
    for (auto& element : this->Elements)
    {
        auto& coordinateElement = element.CoordinateElement();
        for (int iNode = 0; iNode < coordinateElement.GetNumNodes(); ++iNode)
        {
            Eigen::VectorXd globalNodeCoords = coordinateElement.GetNode(iNode).GetValues(); 
            if (std::abs(globalNodeCoords[directionComponent] - axisOffset) < tol)
                group.Add(coordinateElement.GetNode(iNode));
        }
    }
    return group;
}

Groups::Group<NodeSimple> MeshFem::NodesAtAxis(eDirection direction, DofType dofType, double axisOffset /* = 0.*/,
                                               double tol /* = 1.e-10 */)
{
    Groups::Group<NodeSimple> group;
    const int directionComponent = ToComponentIndex(direction);
    for (auto& element : this->Elements)
    {
        auto& dofElement = element.DofElement(dofType);
        const auto& dofInterpolation = dofElement.Interpolation();
        for (int iNode = 0; iNode < dofInterpolation.GetNumNodes(); ++iNode)
        {
            NaturalCoords dofNodeCoords = dofInterpolation.GetLocalCoords(iNode);
            Eigen::VectorXd globalNodeCoords = Interpolate(element.CoordinateElement(), dofNodeCoords);

            if (std::abs(globalNodeCoords[directionComponent] - axisOffset) < tol)
                group.Add(dofElement.GetNode(iNode));
        }
    }
    return group;
}

Groups::Group<NodeSimple> MeshFem::NodesTotal()
{
    Groups::Group<NodeSimple> group;
    for (auto& element : this->Elements)
        for (int iNode = 0; iNode < element.CoordinateElement().Interpolation().GetNumNodes(); ++iNode)
            group.Add(element.CoordinateElement().GetNode(iNode));
    return group;
}

Groups::Group<NodeSimple> MeshFem::NodesTotal(DofType d)
{
    Groups::Group<NodeSimple> group;
    for (auto& element : this->Elements)
        for (int iNode = 0; iNode < element.DofElement(d).Interpolation().GetNumNodes(); ++iNode)
            group.Add(element.DofElement(d).GetNode(iNode));
    return group;
}