#include <iostream>

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#endif // ENABLE_SERIALIZATION

#include "mechanics/integrationtypes/IntegrationTypeBase.h"

void NuTo::IntegrationTypeBase::Info(int rVerboseLevel) const
{
    std::cout << IntegrationTypeToString(GetEnumType()) << std::endl;
    if (rVerboseLevel > 2)
    {
        for (int count = 0; count < GetNumIntegrationPoints(); count++)
        {
            std::cout << "    IP " << count << " weight " << GetIntegrationPointWeight(count) << std::endl;
            std::cout << "        coordinates ";
            std::cout << GetLocalIntegrationPointCoordinates(count);
        }
    }
}

Eigen::MatrixXd NuTo::IntegrationTypeBase::GetNaturalIntegrationPointCoordinates() const
{
    throw MechanicsException(__PRETTY_FUNCTION__, "Not implemented in base class.");
}

NuTo::IntegrationTypeBase::IpCellInfo NuTo::IntegrationTypeBase::GetVisualizationCells() const
{
    unsigned int NumVisualizationPoints;
    std::vector<double> VisualizationPointLocalCoordinates;
    unsigned int NumVisualizationCells;
    std::vector<NuTo::eCellTypes> VisualizationCellType;
    std::vector<unsigned int> VisualizationCellsIncidence;
    std::vector<unsigned int> VisualizationCellsIP;

    GetVisualizationCells(NumVisualizationPoints, VisualizationPointLocalCoordinates, NumVisualizationCells,
                          VisualizationCellType, VisualizationCellsIncidence, VisualizationCellsIP);
    IpCellInfo ipCellInfo;
    if (NumVisualizationCells == 0)
        return ipCellInfo;
 

    // transform cell vertex coordinates
    const int dim = VisualizationPointLocalCoordinates.size() / NumVisualizationPoints;
    Eigen::MatrixXd visualizationPointNaturalCoordinates = Eigen::MatrixXd::Map(VisualizationPointLocalCoordinates.data(), dim, NumVisualizationPoints);
    
    ipCellInfo.cellVertices.resize(NumVisualizationPoints);
    for (int i = 0; i < NumVisualizationPoints; ++i)
        ipCellInfo.cellVertices[i] = visualizationPointNaturalCoordinates.col(i);

    // transform cells
    ipCellInfo.cells.resize(NumVisualizationCells);
    auto it = VisualizationCellsIncidence.begin();
    for (int i = 0; i < NumVisualizationCells; ++i)
    {
        CellInfo& cellInfo = ipCellInfo.cells[i];
        cellInfo.cellType = VisualizationCellType[i];
        cellInfo.ipId = VisualizationCellsIP[i];

        const int numPoints = Visualize::GetNumPoints(cellInfo.cellType);
        cellInfo.pointIds = std::vector<int>(it, it + numPoints);
        std::advance(it, numPoints);
    }

    return ipCellInfo;
}

void NuTo::IntegrationTypeBase::AddIntegrationPoints(std::vector<std::vector<double>>& rArea,
                                                     const unsigned short rOrder)
{
    throw MechanicsException(__PRETTY_FUNCTION__,
                             "Cannot add an IP to integration type " + IntegrationTypeToString(GetEnumType()) + ".");
}

void NuTo::IntegrationTypeBase::AddIntegrationPoint(const IntegrationPointBase& rIp)
{
    throw MechanicsException(__PRETTY_FUNCTION__,
                             "Cannot add an IP to integration type " + IntegrationTypeToString(GetEnumType()) + ".");
}

void NuTo::IntegrationTypeBase::DeleteIntegrationPoint(const int rIpNum)
{
    throw MechanicsException(__PRETTY_FUNCTION__,
                             "Cannot delete an IP to integration type " + IntegrationTypeToString(GetEnumType()) + ".");
}

#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::IntegrationTypeBase)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(NuTo::IntegrationTypeBase)
#endif // ENABLE_SERIALIZATION
