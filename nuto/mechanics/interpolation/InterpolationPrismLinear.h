#pragma once
#include "nuto/mechanics/interpolation/InterpolationSimple.h"
#include "nuto/math/shapes/Prism.h"

namespace NuTo
{
class InterpolationPrismLinear : public InterpolationSimple
{
public:
    static Eigen::Matrix<double, 3, 1> LocalCoords(int rNodeIndex);

    static Eigen::Matrix<double, 6, 1> ShapeFunctions(const Eigen::VectorXd& rCoordinates);

    static Eigen::Matrix<double, 6, 3> DerivativeShapeFunctions(const Eigen::VectorXd& rCoordinates);

    std::unique_ptr<InterpolationSimple> Clone() const override;

    Eigen::VectorXd GetShapeFunctions(const NaturalCoords& naturalIpCoords) const override;

    Eigen::MatrixXd GetDerivativeShapeFunctions(const NaturalCoords& naturalIpCoords) const override;

    NaturalCoords GetLocalCoords(int nodeId) const override;

    int GetNumNodes() const override;

    const Shape& GetShape() const override;

    std::vector<int> EdgeNodeIds(int edgeIndex) const override;

    int NumEdges() const override
    {
        return 9;
    }

    int NumFaces() const override
    {
        return 5;
    }

    virtual std::unique_ptr<InterpolationSimple> EdgeInterpolation(int /* edgeIndex*/) const override;

    virtual std::vector<int> FaceNodeIds(int faceIndex) const override;

    virtual std::unique_ptr<InterpolationSimple> FaceInterpolation(int /* faceIndex*/) const override;

private:
    Prism mShape;
};
} /* NuTo */