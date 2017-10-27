#include "visualize/Point.h"
#include "base/Exception.h"

using namespace NuTo::Visualize;

Point::Point(Eigen::Vector3d coordinates)
    : mCoordinates(coordinates)
{
}

Point::Point(Eigen::VectorXd coordinates)
    : mCoordinates(Eigen::Vector3d::Zero())
{
    const int dim = coordinates.rows();
    mCoordinates.segment(0, dim) = coordinates;
}

void Point::SetData(int dataIndex, Eigen::VectorXd data)
{
    if (dataIndex >= static_cast<int>(mData.size()))
        mData.resize(dataIndex + 1);
    mData[dataIndex] = data;
}

const Eigen::Vector3d& Point::GetCoordinates() const
{
    return mCoordinates;
}

const Eigen::VectorXd& Point::GetData(int dataIndex) const
{
    return mData[dataIndex];
}
