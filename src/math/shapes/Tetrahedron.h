#pragma once

#include "Shape.h"

namespace NuTo
{

class Tetrahedron : public Shape
{
public:
    eShape Enum() const override
    {
        return eShape::Tetrahedron;
    }

    bool IsWithinShape(Eigen::VectorXd xi) const override
    {
        double x = xi[0];
        double y = xi[1];
        double z = xi[2];
        return (0.<x)&&(x<1.)&&(0.<y)&&(y<1.)&&(0.<z)&&(z<1.)&&(x+y+z<1.);
    }
};

} // namespace NuTo
