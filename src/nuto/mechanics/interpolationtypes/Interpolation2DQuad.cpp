/*
 * InterpolationType2DQuad.cpp
 *
 *  Created on: 24 Apr 2015
 *      Author: ttitsche
 */

#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/elements/ElementShapeFunctions.h"
#include "nuto/mechanics/integrationtypes/IntegrationTypeEnum.h"
#include "nuto/mechanics/interpolationtypes/InterpolationTypeEnum.h"
#include "nuto/mechanics/interpolationtypes/Interpolation2DQuad.h"

NuTo::Interpolation2DQuad::Interpolation2DQuad(NuTo::Node::eDof rDofType, NuTo::Interpolation::eTypeOrder rTypeOrder, int rDimension) :
        Interpolation2D::Interpolation2D(rDofType, rTypeOrder, rDimension)
{
    Initialize();
}

NuTo::eIntegrationType NuTo::Interpolation2DQuad::GetStandardIntegrationType() const
{
    switch (mTypeOrder)
    {
    case NuTo::Interpolation::eTypeOrder::EQUIDISTANT1:
        return NuTo::eIntegrationType::IntegrationType2D4NGauss4Ip;
    case NuTo::Interpolation::eTypeOrder::EQUIDISTANT2:
        return NuTo::eIntegrationType::IntegrationType2D4NGauss4Ip;
    case NuTo::Interpolation::eTypeOrder::LOBATTO2:
        return NuTo::eIntegrationType::IntegrationType2D4NLobatto9Ip;
    case NuTo::Interpolation::eTypeOrder::LOBATTO3:
        return NuTo::eIntegrationType::IntegrationType2D4NLobatto16Ip;
    case NuTo::Interpolation::eTypeOrder::LOBATTO4:
        return NuTo::eIntegrationType::IntegrationType2D4NLobatto25Ip;

    default:
        throw MechanicsException(__PRETTY_FUNCTION__, "Interpolation for exact integration of " + Interpolation::TypeOrderToString(mTypeOrder) + " not implemented");
    }
}

Eigen::VectorXd NuTo::Interpolation2DQuad::CalculateShapeFunctions(const Eigen::VectorXd& rCoordinates) const
{
    switch (mTypeOrder)
    {
    case NuTo::Interpolation::eTypeOrder::EQUIDISTANT1:
        return ShapeFunctions2D::ShapeFunctionsQuadOrder1(rCoordinates);
    case NuTo::Interpolation::eTypeOrder::EQUIDISTANT2:
        return ShapeFunctions2D::ShapeFunctionsQuadOrder2(rCoordinates);
    case NuTo::Interpolation::eTypeOrder::LOBATTO2:
        return ShapeFunctions2D::ShapeFunctionsQuadSpectralOrder2(rCoordinates);
    case NuTo::Interpolation::eTypeOrder::LOBATTO3:
        return ShapeFunctions2D::ShapeFunctionsQuadSpectralOrder3(rCoordinates);
    case NuTo::Interpolation::eTypeOrder::LOBATTO4:
        return ShapeFunctions2D::ShapeFunctionsQuadSpectralOrder4(rCoordinates);
    default:
        throw MechanicsException(__PRETTY_FUNCTION__, "Interpolation order for " + Interpolation::TypeOrderToString(mTypeOrder) + " not implemented");
    }
}

Eigen::MatrixXd NuTo::Interpolation2DQuad::CalculateDerivativeShapeFunctionsNatural(const Eigen::VectorXd& rCoordinates) const
{
    switch (mTypeOrder)
    {
    case NuTo::Interpolation::eTypeOrder::EQUIDISTANT1:
        return ShapeFunctions2D::DerivativeShapeFunctionsQuadOrder1(rCoordinates);
    case NuTo::Interpolation::eTypeOrder::EQUIDISTANT2:
        return ShapeFunctions2D::DerivativeShapeFunctionsQuadOrder2(rCoordinates);
    case NuTo::Interpolation::eTypeOrder::LOBATTO2:
        return ShapeFunctions2D::DerivativeShapeFunctionsQuadSpectralOrder2(rCoordinates);
    case NuTo::Interpolation::eTypeOrder::LOBATTO3:
        return ShapeFunctions2D::DerivativeShapeFunctionsQuadSpectralOrder3(rCoordinates);
    case NuTo::Interpolation::eTypeOrder::LOBATTO4:
        return ShapeFunctions2D::DerivativeShapeFunctionsQuadSpectralOrder4(rCoordinates);

    default:
        throw MechanicsException(__PRETTY_FUNCTION__, "Interpolation order for " + Interpolation::TypeOrderToString(mTypeOrder) + " not implemented");
    }
}

Eigen::VectorXd NuTo::Interpolation2DQuad::CalculateNaturalNodeCoordinates(int rNodeIndexDof) const
{
    switch (mTypeOrder)
    {
    case NuTo::Interpolation::eTypeOrder::EQUIDISTANT1:
        return ShapeFunctions2D::NodeCoordinatesQuadOrder1(rNodeIndexDof);
    case NuTo::Interpolation::eTypeOrder::EQUIDISTANT2:
        return ShapeFunctions2D::NodeCoordinatesQuadOrder2(rNodeIndexDof);
    case NuTo::Interpolation::eTypeOrder::LOBATTO2:
        return ShapeFunctions2D::NodeCoordinatesQuadSpectralOrder2(rNodeIndexDof);
    case NuTo::Interpolation::eTypeOrder::LOBATTO3:
        return ShapeFunctions2D::NodeCoordinatesQuadSpectralOrder3(rNodeIndexDof);
    case NuTo::Interpolation::eTypeOrder::LOBATTO4:
        return ShapeFunctions2D::NodeCoordinatesQuadSpectralOrder4(rNodeIndexDof);

    default:
        throw MechanicsException(__PRETTY_FUNCTION__, "Node arrangement for " + Interpolation::TypeOrderToString(mTypeOrder) + " not implemented");
    }
}

Eigen::VectorXd NuTo::Interpolation2DQuad::CalculateNaturalSurfaceCoordinates(const Eigen::VectorXd& rNaturalSurfaceCoordinates, int rSurface) const
{
    assert(rNaturalSurfaceCoordinates.rows() == 1);
    switch (rSurface)
    {
    case 0:
        return Eigen::Vector2d(rNaturalSurfaceCoordinates(0), -1.);
    case 1:
        return Eigen::Vector2d(1., rNaturalSurfaceCoordinates(0));
    case 2:
        return Eigen::Vector2d(-rNaturalSurfaceCoordinates(0), 1.);
    case 3:
        return Eigen::Vector2d(-1., -rNaturalSurfaceCoordinates(0));
    default:
        throw MechanicsException(__PRETTY_FUNCTION__, "QUAD2D has exactly four surfaces, 0 to 3. You tried to access " + std::to_string(rSurface) + ".");
    }
}

Eigen::MatrixXd NuTo::Interpolation2DQuad::CalculateDerivativeNaturalSurfaceCoordinates(const Eigen::VectorXd& rNaturalSurfaceCoordinates, int rSurface) const
{
    assert(rNaturalSurfaceCoordinates.rows() == 1);
    switch (rSurface)
    {
    case 0:
        return Eigen::Vector2d(1, 0);
    case 1:
        return Eigen::Vector2d(0, 1);
    case 2:
        return Eigen::Vector2d(-1, 0);
    case 3:
        return Eigen::Vector2d(0, -1);
    default:
        throw MechanicsException(__PRETTY_FUNCTION__, "QUAD2D has exactly four surfaces, 0 to 3. You tried to access " + std::to_string(rSurface) + ".");
    }
}

int NuTo::Interpolation2DQuad::CalculateNumNodes() const
{
    switch (mTypeOrder)
    {
    case NuTo::Interpolation::eTypeOrder::EQUIDISTANT1:
        return 4;
    case NuTo::Interpolation::eTypeOrder::EQUIDISTANT2:
        return 8;
    case NuTo::Interpolation::eTypeOrder::LOBATTO2:
        return 9;
    case NuTo::Interpolation::eTypeOrder::LOBATTO3:
        return 16;
    case NuTo::Interpolation::eTypeOrder::LOBATTO4:
        return 25;

    default:
        throw MechanicsException(__PRETTY_FUNCTION__, "Interpolation type and order " + Interpolation::TypeOrderToString(mTypeOrder) + " not implemented");
    }
}

void NuTo::Interpolation2DQuad::UpdateNodeIndices(const std::vector<Eigen::VectorXd> &rNodeCoordinates, std::function<bool(const Eigen::VectorXd& rC1, const Eigen::VectorXd& rC2)> rFunction)
{
    switch (mTypeOrder)
    {
    case NuTo::Interpolation::eTypeOrder::EQUIDISTANT1:
    case NuTo::Interpolation::eTypeOrder::EQUIDISTANT2:
    case NuTo::Interpolation::eTypeOrder::LOBATTO2:
    case NuTo::Interpolation::eTypeOrder::LOBATTO3:
    case NuTo::Interpolation::eTypeOrder::LOBATTO4:
    {
        int count = 0;
        mNodeIndices.resize(GetNumNodes());
        for (int iNode = 0; iNode < GetNumNodes(); ++iNode)
        {
            Eigen::VectorXd coordinates = GetNaturalNodeCoordinates(iNode);
            for(unsigned int iExistingNode = 0; iExistingNode < rNodeCoordinates.size(); ++iExistingNode)
            {
                if(rFunction(coordinates, rNodeCoordinates[iExistingNode]))
                {
                    mNodeIndices[iNode] = iExistingNode;
                    count++;
                }
            }
        }
        if(count < mNumNodes) throw MechanicsException(__PRETTY_FUNCTION__, "Not all nodes found!");
        break;
    }
    default:
        throw MechanicsException(__PRETTY_FUNCTION__, "Interpolation for exact integration of " + Interpolation::TypeOrderToString(mTypeOrder) + " not implemented");
    }
}

#ifdef ENABLE_SERIALIZATION
template<class Archive>
void NuTo::Interpolation2DQuad::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize Interpolation2D\n";
#endif
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Interpolation2D);
#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize Interpolation2D\n";
#endif
}
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::Interpolation2DQuad)
#endif