#include "nuto/base/Exception.h"
#include "InterpolationTrussLinear.h"

using namespace NuTo;

std::unique_ptr<InterpolationSimple> InterpolationTrussLinear::Clone() const
{
    return std::make_unique<InterpolationTrussLinear>(*this);
}

Eigen::VectorXd InterpolationTrussLinear::GetShapeFunctions(const NaturalCoords& naturalIpCoords) const
{
    return ShapeFunctions(naturalIpCoords);
}

Eigen::MatrixXd InterpolationTrussLinear::GetDerivativeShapeFunctions(const NaturalCoords&) const
{
    return DerivativeShapeFunctions();
}

NaturalCoords InterpolationTrussLinear::GetLocalCoords(int nodeId) const
{
    return LocalCoords(nodeId);
}

int InterpolationTrussLinear::GetNumNodes() const
{
    return 2;
}

const Shape& InterpolationTrussLinear::GetShape() const
{
    return mShape;
}

Eigen::Matrix<double, 1, 1> InterpolationTrussLinear::LocalCoords(int rNodeIndex)
{
    switch (rNodeIndex)
    {
    case 0:
        return Eigen::Matrix<double, 1, 1>::Constant(-1.);
    case 1:
        return Eigen::Matrix<double, 1, 1>::Constant(1.);
    default:
        throw NuTo::Exception(__PRETTY_FUNCTION__, "node index out of range (0..1)");
    }
}

Eigen::Matrix<double, 2, 1> InterpolationTrussLinear::ShapeFunctions(const Eigen::VectorXd& rCoordinates)
{
    return Eigen::Vector2d(0.5 * (1. - rCoordinates[0]), 0.5 * (1. + rCoordinates[0]));
}

Eigen::Matrix<double, 2, 1> InterpolationTrussLinear::DerivativeShapeFunctions()
{
    return Eigen::Vector2d(-0.5, 0.5);
}

std::vector<int> InterpolationTrussLinear::EdgeNodeIds(int edgeIndex) const
{
    throw Exception(__PRETTY_FUNCTION__, "Not implemented");
}

std::unique_ptr<InterpolationSimple> InterpolationTrussLinear::EdgeInterpolation(int /* edgeIndex*/) const
{
    throw Exception(__PRETTY_FUNCTION__, "Not implemented");
}

std::vector<int> InterpolationTrussLinear::FaceNodeIds(int /* faceIndex */) const
{
    throw Exception(__PRETTY_FUNCTION__, "Not implemented");
}

std::unique_ptr<InterpolationSimple> InterpolationTrussLinear::FaceInterpolation(int /* faceIndex*/) const
{
    throw Exception(__PRETTY_FUNCTION__, "Not implemented");
}
