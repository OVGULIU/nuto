#include "BoostUnitTest.h"
#include "InterpolationTests.h"
#include "nuto/mechanics/interpolation/InterpolationTriangle3rdOrder.h"

std::vector<Eigen::VectorXd> GetTestPoints()
{
    return {Eigen::Vector2d({1. / 6., 1. / 6.}), Eigen::Vector2d({4. / 6., 1. / 6.}),
            Eigen::Vector2d({1. / 6., 4. / 6.})};
}

BOOST_AUTO_TEST_CASE(InterpolationTriangle3rdOrder)
{
    NuTo::Test::RunTests<NuTo::InterpolationTriangle3rdOrder>(GetTestPoints());
}
