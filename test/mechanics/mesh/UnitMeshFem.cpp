#include "nuto/mechanics/mesh/GeometryMeshFem.h"
#include "nuto/mechanics/mesh/UnitMeshFem.h"
#include "nuto/mechanics/cell/Jacobian.h"
#include "BoostUnitTest.h"

void CheckJacobians(NuTo::GeometryMeshFem& mesh)
{
    int dim = mesh.GetElements()[0].GetNode(0).GetCoordinates().rows();
    Eigen::VectorXd ip = Eigen::VectorXd::Zero(dim);
    for (auto& element : mesh.GetElements())
    {
        auto d_dxi = element.GetDerivativeShapeFunctions(ip);
        auto x = element.ExtractCoordinates();
        auto J = NuTo::Jacobian(x, d_dxi);
        BOOST_CHECK_GT(J.Det(), 0.);
    }
}

void Check2DMesh(NuTo::GeometryMeshFem& mesh)
{
    BOOST_CHECK_EQUAL(mesh.NumNodes(), 3 * 8);

    BOOST_CHECK_NO_THROW(mesh.NodeAtCoordinate(Eigen::Vector2d(0., 0.)));
    BOOST_CHECK_NO_THROW(mesh.NodeAtCoordinate(Eigen::Vector2d(1., 1.)));

    BOOST_CHECK_NO_THROW(mesh.NodeAtCoordinate(Eigen::Vector2d(1. / 2., 1. / 7.)));
    BOOST_CHECK_NO_THROW(mesh.NodeAtCoordinate(Eigen::Vector2d(1. / 2., 5. / 7.)));

    CheckJacobians(mesh);

    auto f = [](Eigen::VectorXd coords) // transforms mesh to (42,4) -- (44, 11)
    {
        Eigen::VectorXd newCoords(2);
        newCoords[0] = 42 + coords[0] * 2;
        newCoords[1] = 4 + coords[1] * 7;
        return newCoords;
    };

    NuTo::GeometryMeshFem transformedMesh = NuTo::UnitMeshFem::Transform(std::move(mesh), f);

    BOOST_CHECK_NO_THROW(transformedMesh.NodeAtCoordinate(Eigen::Vector2d(42., 4.)));
    BOOST_CHECK_NO_THROW(transformedMesh.NodeAtCoordinate(Eigen::Vector2d(44., 11.)));

    BOOST_CHECK_NO_THROW(transformedMesh.NodeAtCoordinate(Eigen::Vector2d(43., 5.)));
    BOOST_CHECK_NO_THROW(transformedMesh.NodeAtCoordinate(Eigen::Vector2d(43., 9.)));
}

BOOST_AUTO_TEST_CASE(MeshTrusses)
{
    constexpr int numElements = 15;
    auto mesh = NuTo::UnitMeshFem::CreateLines(numElements);
    BOOST_CHECK_EQUAL(mesh.GetElements().Size(), numElements);
    BOOST_CHECK_EQUAL(mesh.NumNodes(), numElements + 1);

    auto IsWholeNumber = [](double d, double eps = 1.e-12) { return std::abs(d - std::floor(d)) < eps; };

    for (size_t i = 0; i < mesh.NumNodes(); i++)
    {
        auto& node = mesh.GetNode(i);
        BOOST_CHECK(IsWholeNumber(node.GetCoordinates()[0] * numElements));
        BOOST_CHECK_LE(node.GetCoordinates()[0], 1.0);
        BOOST_CHECK_GE(node.GetCoordinates()[0], 0.0);
    }

    for (const auto& element : mesh.GetElements())
    {
        BOOST_CHECK_LT(element.GetNode(0).GetCoordinates()[0], element.GetNode(1).GetCoordinates()[0]);
    }
}

BOOST_AUTO_TEST_CASE(MeshQuad)
{
    auto mesh = NuTo::UnitMeshFem::CreateQuads(2, 7);
    BOOST_CHECK_EQUAL(mesh.GetElements().Size(), 2 * 7);
    Check2DMesh(mesh);
}

BOOST_AUTO_TEST_CASE(MeshTriangle)
{
    auto mesh = NuTo::UnitMeshFem::CreateTriangles(2, 7);
    BOOST_CHECK_EQUAL(mesh.GetElements().Size(), 2 * 7 * 2);
    Check2DMesh(mesh);
}

BOOST_AUTO_TEST_CASE(MeshBrick)
{
    auto mesh = NuTo::UnitMeshFem::CreateBricks(2, 7, 3);
    BOOST_CHECK_EQUAL(mesh.GetElements().Size(), 2 * 7 * 3);
    BOOST_CHECK_EQUAL(mesh.NumNodes(), 3 * 8 * 4);
    BOOST_CHECK_NO_THROW(mesh.NodeAtCoordinate(Eigen::Vector3d(0, 0, 0)));
    BOOST_CHECK_NO_THROW(mesh.NodeAtCoordinate(Eigen::Vector3d(1, 1, 1)));
    CheckJacobians(mesh);
}

BOOST_AUTO_TEST_CASE(MeshValidAfterTransform)
{
    auto mesh = NuTo::UnitMeshFem::CreateQuads(1, 1);
    Eigen::VectorXd expected(8);
    expected << 0, 0, 1, 0, 1, 1, 0, 1;

    auto& coordinateElement = mesh.GetElements()[0];
    BoostUnitTest::CheckEigenMatrix(coordinateElement.ExtractCoordinates(), expected);

    auto f = [](Eigen::VectorXd coords) { return Eigen::Vector2d(coords[0] * 4, coords[1] * 42); };

    NuTo::GeometryMeshFem transformedMesh = NuTo::UnitMeshFem::Transform(std::move(mesh), f);
    auto& transformedCoordinateElement = transformedMesh.GetElements()[0];
    expected << 0, 0, 4, 0, 4, 42, 0, 42;
    BoostUnitTest::CheckEigenMatrix(transformedCoordinateElement.ExtractCoordinates(), expected);

    transformedMesh.GetNode(0).SetCoordinate(0, 6174);
    expected << 6174, 0, 4, 0, 4, 42, 0, 42;
    BoostUnitTest::CheckEigenMatrix(transformedCoordinateElement.ExtractCoordinates(), expected);
}

// This test is related to our github issue #148. Visit github to read about the details
BOOST_AUTO_TEST_CASE(MeshMovabilityError)
{
    NuTo::GeometryMeshFem mesh = NuTo::UnitMeshFem::CreateLines(1);
    {
        NuTo::GeometryMeshFem tempMesh = NuTo::UnitMeshFem::CreateLines(1);
        mesh = std::move(tempMesh);
    }
    auto& coordinateElement = mesh.GetElements()[0];
    coordinateElement.GetDofDimension();
}
