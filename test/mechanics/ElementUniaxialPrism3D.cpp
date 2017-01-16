/*
 * ElementUniaxialPrism3D.cpp
 *
 *  Created on: 12 January 2017
 *      Author: Thomas Titscher
 */

#include "../test/mechanics/ElementUniaxialTest.h"

std::string directory = "";


void Run(NuTo::Interpolation::eTypeOrder rTypeOrder)
{
    NuToTest::ElementUniaxialTest test;

#ifdef ENABLE_VISUALIZE
    test.visualizationDirectory = directory;
#endif

    NuTo::Structure myStructure(3);
    myStructure.SetShowTime(false);

    int numElementsX = 1;
    int numElementsY = 1;
    int numElementsZ = 1;

    //create nodes
    int numNodesX = numElementsX+1;
    int numNodesY = numElementsY+1;
    int numNodesZ = numElementsZ+1;

    double deltaX = test.lX/(numElementsX);
    double deltaY = test.lY/(numElementsY);
    double deltaZ = test.lZ/(numElementsZ);

    int nodeNum = 0;
    for (int iZ=0; iZ<numNodesZ; iZ++)
        for (int iY=0; iY<numNodesY; iY++)
            for (int iX=0; iX<numNodesX; iX++)
            {
                myStructure.NodeCreate(nodeNum, Eigen::Vector3d(iX*deltaX, iY*deltaY, iZ*deltaZ));
                nodeNum++;
            }

    int myInterpolationType = myStructure.InterpolationTypeCreate("PRISM3D");
    myStructure.InterpolationTypeAdd(myInterpolationType, NuTo::Node::eDof::COORDINATES, NuTo::Interpolation::eTypeOrder::EQUIDISTANT1);
    myStructure.InterpolationTypeAdd(myInterpolationType, NuTo::Node::eDof::DISPLACEMENTS, rTypeOrder);

    std::vector<int> nodes(6);
    for (int iZ=0; iZ<numElementsZ; iZ++)
        for (int iY=0; iY<numElementsY; iY++)
            for (int iX=0; iX<numElementsX; iX++)
            {
                nodes[0] = iX   +  iY    * numNodesX +  iZ    * numNodesX * numNodesY;
                nodes[1] = iX+1 +  iY    * numNodesX +  iZ    * numNodesX * numNodesY;
                nodes[2] = iX+1 + (iY+1) * numNodesX +  iZ    * numNodesX * numNodesY;
                nodes[3] = iX   +  iY    * numNodesX + (iZ+1) * numNodesX * numNodesY;
                nodes[4] = iX+1 +  iY    * numNodesX + (iZ+1) * numNodesX * numNodesY;
                nodes[5] = iX+1 + (iY+1) * numNodesX + (iZ+1) * numNodesX * numNodesY;
                myStructure.ElementCreate(myInterpolationType, nodes);

                nodes[0] = iX   +  iY    * numNodesX +  iZ    * numNodesX * numNodesY;
                nodes[1] = iX+1 + (iY+1) * numNodesX +  iZ    * numNodesX * numNodesY;
                nodes[2] = iX   + (iY+1) * numNodesX +  iZ    * numNodesX * numNodesY;
                nodes[3] = iX   +  iY    * numNodesX + (iZ+1) * numNodesX * numNodesY;
                nodes[4] = iX+1 + (iY+1) * numNodesX + (iZ+1) * numNodesX * numNodesY;
                nodes[5] = iX   + (iY+1) * numNodesX + (iZ+1) * numNodesX * numNodesY;
                myStructure.ElementCreate(myInterpolationType, nodes);
            }


    myStructure.SetVerboseLevel(10);
    myStructure.SetShowTime(true);
    myStructure.ElementTotalConvertToInterpolationType();

    int allElements = myStructure.GroupCreate("Elements");
    myStructure.GroupAddElementFromType(allElements, myInterpolationType);
    double volume = myStructure.ElementGroupGetVolume(allElements);
    std::cout << "######### VOLUME: " << volume << std::endl;


    int mySection = myStructure.SectionCreate("VOLUME");
    myStructure.ElementTotalSetSection(mySection);

    test.Run(myStructure);
}

int main(int argc, char* argv[])
{

    boost::filesystem::path path = boost::filesystem::system_complete(boost::filesystem::path( argv[0] ));
    directory = path.parent_path().string();

    try
    {
        Run(NuTo::Interpolation::eTypeOrder::EQUIDISTANT1);
        Run(NuTo::Interpolation::eTypeOrder::EQUIDISTANT2);
    }
    catch (NuTo::Exception& e)
    {
        std::cout << "## Test failed ##" << std::endl;
        std::cout << e.ErrorMessage();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
