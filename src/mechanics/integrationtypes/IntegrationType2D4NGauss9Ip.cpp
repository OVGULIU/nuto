#ifdef ENABLE_VISUALIZE
#include "visualize/VisualizeEnum.h"
#endif // ENABLE_VISUALIZE

#include "mechanics/integrationtypes/IntegrationType2D4NGauss9Ip.h"
#include <cassert>


//! @brief constructor
NuTo::IntegrationType2D4NGauss9Ip::IntegrationType2D4NGauss9Ip()
{
}

//! @brief returns the local coordinates of an integration point
//! @param rIpNum integration point (counting from zero)
//! @param rCoordinates (result)
Eigen::VectorXd NuTo::IntegrationType2D4NGauss9Ip::GetLocalIntegrationPointCoordinates(int rIpNum) const
{
    assert(rIpNum>=0 && rIpNum<9);
    switch (rIpNum)
    {
    case 0 : return Eigen::Vector2d({ -0.774596669241483, -0.774596669241483});
    case 1 : return Eigen::Vector2d({  0.774596669241483, -0.774596669241483});
    case 2 : return Eigen::Vector2d({  0.774596669241483,  0.774596669241483});
    case 3 : return Eigen::Vector2d({ -0.774596669241483,  0.774596669241483});
    case 4 : return Eigen::Vector2d({  0.0,               -0.774596669241483});
    case 5 : return Eigen::Vector2d({  0.774596669241483,  0.0});
    case 6 : return Eigen::Vector2d({  0.0,                0.774596669241483});
    case 7 : return Eigen::Vector2d({ -0.774596669241483,  0.0});
    case 8 : return Eigen::Vector2d({  0.0,                0.0});
    default:
        throw MechanicsException("[NuTo::IntegrationType2D4NGauss9Ip::GetLocalIntegrationPointCoordinates] Ip number out of range.");
    }
}


//! @brief returns the total number of integration points for this integration type
//! @return number of integration points
int NuTo::IntegrationType2D4NGauss9Ip::GetNumIntegrationPoints()const
{
    return 9;
}

//! @brief returns the weight of an integration point
//! @param rIpNum integration point (counting from zero)
//! @return weight of integration points
double NuTo::IntegrationType2D4NGauss9Ip::GetIntegrationPointWeight(int rIpNum)const
{
    assert(rIpNum>=0 && rIpNum<9);
    switch (rIpNum)
    {
    case 0 :
        return 0.308641975; // 5/9 * 5/9
        break;
    case 1 :
        return 0.308641975; // 5/9 * 5/9
        break;
    case 2 :
        return 0.308641975; // 5/9 * 5/9
        break;
    case 3 :
        return 0.308641975; // 5/9 * 5/9
        break;
    case 4 :
        return 0.493827160; // 5/9 * 8/9
        break;
    case 5 :
        return 0.493827160; // 5/9 * 8/9
        break;
    case 6 :
        return 0.493827160; // 5/9 * 8/9
        break;
    case 7 :
        return 0.493827160; // 5/9 * 8/9
        break;
    case 8 :
        return 0.790123456; // 8/9 * 8/9
        break;
    default:
        throw MechanicsException("[NuTo::IntegrationType2D4NGauss9Ip::GetLocalIntegrationPointCoordinates] Ip number out of range.");
    }
}

#ifdef ENABLE_VISUALIZE
void NuTo::IntegrationType2D4NGauss9Ip::GetVisualizationCells(
    unsigned int& NumVisualizationPoints,
    std::vector<double>& VisualizationPointLocalCoordinates,
    unsigned int& NumVisualizationCells,
    std::vector<NuTo::eCellTypes>& VisualizationCellType,
    std::vector<unsigned int>& VisualizationCellsIncidence,
    std::vector<unsigned int>& VisualizationCellsIP) const
{
    NumVisualizationPoints = 16;

    // first row
    // Point 0
    VisualizationPointLocalCoordinates.push_back(-1);
    VisualizationPointLocalCoordinates.push_back(-1);

    // Point 1
    VisualizationPointLocalCoordinates.push_back(-1./3);
    VisualizationPointLocalCoordinates.push_back(-1);

    // Point 2
    VisualizationPointLocalCoordinates.push_back(+1./3);
    VisualizationPointLocalCoordinates.push_back(-1);

    // Point 3
    VisualizationPointLocalCoordinates.push_back(1);
    VisualizationPointLocalCoordinates.push_back(-1);

    // second row
    // Point 4
    VisualizationPointLocalCoordinates.push_back(-1);
    VisualizationPointLocalCoordinates.push_back(-1./3);

    // Point 5
    VisualizationPointLocalCoordinates.push_back(-1./3);
    VisualizationPointLocalCoordinates.push_back(-1./3);

    // Point 6
    VisualizationPointLocalCoordinates.push_back(+1./3);
    VisualizationPointLocalCoordinates.push_back(-1./3);

    // Point 7
    VisualizationPointLocalCoordinates.push_back(1);
    VisualizationPointLocalCoordinates.push_back(-1./3);

    // third row
    // Point 8
    VisualizationPointLocalCoordinates.push_back(-1);
    VisualizationPointLocalCoordinates.push_back(1./3);

    // Point 9
    VisualizationPointLocalCoordinates.push_back(-1./3);
    VisualizationPointLocalCoordinates.push_back(1./3);

    // Point 10
    VisualizationPointLocalCoordinates.push_back(+1./3);
    VisualizationPointLocalCoordinates.push_back(1./3);

    // Point 11
    VisualizationPointLocalCoordinates.push_back(1);
    VisualizationPointLocalCoordinates.push_back(1./3);

    // fourth row
    // Point 12
    VisualizationPointLocalCoordinates.push_back(-1);
    VisualizationPointLocalCoordinates.push_back(1);

    // Point 13
    VisualizationPointLocalCoordinates.push_back(-1./3);
    VisualizationPointLocalCoordinates.push_back(1);

    // Point 14
    VisualizationPointLocalCoordinates.push_back(+1./3);
    VisualizationPointLocalCoordinates.push_back(1);

    // Point 15
    VisualizationPointLocalCoordinates.push_back(1);
    VisualizationPointLocalCoordinates.push_back(1);

    NumVisualizationCells = 9;

    // cell 0
    VisualizationCellType.push_back(NuTo::eCellTypes::QUAD);
    VisualizationCellsIncidence.push_back(0);
    VisualizationCellsIncidence.push_back(1);
    VisualizationCellsIncidence.push_back(5);
    VisualizationCellsIncidence.push_back(4);
    VisualizationCellsIP.push_back(0);

    // cell 4
    VisualizationCellType.push_back(NuTo::eCellTypes::QUAD);
    VisualizationCellsIncidence.push_back(1);
    VisualizationCellsIncidence.push_back(2);
    VisualizationCellsIncidence.push_back(6);
    VisualizationCellsIncidence.push_back(5);
    VisualizationCellsIP.push_back(4);

    // cell 1
    VisualizationCellType.push_back(NuTo::eCellTypes::QUAD);
    VisualizationCellsIncidence.push_back(2);
    VisualizationCellsIncidence.push_back(3);
    VisualizationCellsIncidence.push_back(7);
    VisualizationCellsIncidence.push_back(6);
    VisualizationCellsIP.push_back(1);

    // cell 7
    VisualizationCellType.push_back(NuTo::eCellTypes::QUAD);
    VisualizationCellsIncidence.push_back(4);
    VisualizationCellsIncidence.push_back(5);
    VisualizationCellsIncidence.push_back(9);
    VisualizationCellsIncidence.push_back(8);
    VisualizationCellsIP.push_back(7);

    // cell 8
    VisualizationCellType.push_back(NuTo::eCellTypes::QUAD);
    VisualizationCellsIncidence.push_back(5);
    VisualizationCellsIncidence.push_back(6);
    VisualizationCellsIncidence.push_back(10);
    VisualizationCellsIncidence.push_back(9);
    VisualizationCellsIP.push_back(8);

    // cell 5
    VisualizationCellType.push_back(NuTo::eCellTypes::QUAD);
    VisualizationCellsIncidence.push_back(6);
    VisualizationCellsIncidence.push_back(7);
    VisualizationCellsIncidence.push_back(11);
    VisualizationCellsIncidence.push_back(10);
    VisualizationCellsIP.push_back(5);

    // cell 3
    VisualizationCellType.push_back(NuTo::eCellTypes::QUAD);
    VisualizationCellsIncidence.push_back(8);
    VisualizationCellsIncidence.push_back(9);
    VisualizationCellsIncidence.push_back(13);
    VisualizationCellsIncidence.push_back(12);
    VisualizationCellsIP.push_back(3);

    // cell 6
    VisualizationCellType.push_back(NuTo::eCellTypes::QUAD);
    VisualizationCellsIncidence.push_back(9);
    VisualizationCellsIncidence.push_back(10);
    VisualizationCellsIncidence.push_back(14);
    VisualizationCellsIncidence.push_back(13);
    VisualizationCellsIP.push_back(6);

    // cell 2
    VisualizationCellType.push_back(NuTo::eCellTypes::QUAD);
    VisualizationCellsIncidence.push_back(10);
    VisualizationCellsIncidence.push_back(11);
    VisualizationCellsIncidence.push_back(15);
    VisualizationCellsIncidence.push_back(14);
    VisualizationCellsIP.push_back(2);

}
#endif // ENABLE_VISUALIZE

