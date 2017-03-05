#pragma once
#include <memory>
#include "mechanics/cell/CellData.h"
#include "mechanics/cell/CellIPData.h"

namespace NuTo
{
class Integrand
{
public:
    virtual std::unique_ptr<Integrand> Clone() const = 0;
    virtual ~Integrand()        = default;
    virtual DofContainer<Eigen::VectorXd> Gradient(const CellData&, const CellIPData&) = 0;
};
} /* NuTo */
