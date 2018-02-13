#include "visualize/VoronoiGeometries.h"
#include "visualize/Visualizer.h"
#include "visualize/XMLWriter.h"
#include "TestStructure.h"

using namespace NuTo;


BOOST_AUTO_TEST_CASE(GroupVoronoiTensorProduct2D)
{
    using namespace NuTo::Visualize;
    NuTo::DofType dof("NodeCoordinatesDiv10", 2);
    NuTo::Test::VisualizeTestStructure s(dof);
    auto cells = s.Cells();

    std::string filename = "VoronoiOutput.vtu";
    Visualizer visualize(cells, VoronoiHandler(VoronoiGeometryQuad(2)));
    visualize.WriteVtuFile(filename, false);
    UnstructuredGridCheck::CheckNum(filename, 18, 8);
}