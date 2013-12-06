#include "nuto/math/FullMatrix.h"
#include "nuto/mechanics/structures/unstructured/Structure.h"
#include "nuto/mechanics/MechanicsException.h"

#include "nuto/math/SparseDirectSolverMUMPS.h"
#include "nuto/math/SparseMatrixCSRVector2General.h"


#define PRINTRESULT false

int main()
{
try
{
    //create structure
    NuTo::Structure myStructure(2);

    //create nodes
    NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> nodeCoordinates(2,8);
    nodeCoordinates(0,0) = 0. ;nodeCoordinates(1,0) = 0.;
	nodeCoordinates(0,1) = 10.;nodeCoordinates(1,1) = 0.;
	nodeCoordinates(0,2) = 2. ;nodeCoordinates(1,2) = 2.;
	nodeCoordinates(0,3) = 8. ;nodeCoordinates(1,3) = 3.;
	nodeCoordinates(0,4) = 4. ;nodeCoordinates(1,4) = 7.;
	nodeCoordinates(0,5) = 8. ;nodeCoordinates(1,5) = 7.;
	nodeCoordinates(0,6) = 0. ;nodeCoordinates(1,6) = 10.;
	nodeCoordinates(0,7) = 10.;nodeCoordinates(1,7) = 10.;

    myStructure.NodesCreate("displacements", nodeCoordinates);

    //create element
    NuTo::FullMatrix<int,Eigen::Dynamic,Eigen::Dynamic> elementIncidence(3,10);
	elementIncidence(0,0) = 0; elementIncidence(1,0) = 1; elementIncidence(2,0) = 3;
	elementIncidence(0,1) = 0; elementIncidence(1,1) = 2; elementIncidence(2,1) = 6;
	elementIncidence(0,2) = 0; elementIncidence(1,2) = 3; elementIncidence(2,2) = 2;
	elementIncidence(0,3) = 1; elementIncidence(1,3) = 7; elementIncidence(2,3) = 3;
	elementIncidence(0,4) = 2; elementIncidence(1,4) = 4; elementIncidence(2,4) = 6;
	elementIncidence(0,5) = 2; elementIncidence(1,5) = 3; elementIncidence(2,5) = 4;
	elementIncidence(0,6) = 3; elementIncidence(1,6) = 5; elementIncidence(2,6) = 4;
	elementIncidence(0,7) = 3; elementIncidence(1,7) = 7; elementIncidence(2,7) = 5;
	elementIncidence(0,8) = 4; elementIncidence(1,8) = 5; elementIncidence(2,8) = 6;
	elementIncidence(0,9) = 5; elementIncidence(1,9) = 7; elementIncidence(2,9) = 6;

    myStructure.ElementsCreate("PLANE2D3N", elementIncidence);

    //convert 3N into 10N elements
    int elementGroup1 = myStructure.GroupCreate("Elements");
    myStructure.GroupAddElementFromType(elementGroup1, "PLANE2D3N");
    myStructure.ElementConvertPlane2D3N(elementGroup1,"PLANE2D10N",1e-5,1.);

    //Calculate maximum independent sets for parallelization (openmp)
    myStructure.CalculateMaximumIndependentSets();

    //create constitutive law
    int myMatLin = myStructure.ConstitutiveLawCreate("LinearElasticEngineeringStress");
    myStructure.ConstitutiveLawSetYoungsModulus(myMatLin,10);
    myStructure.ConstitutiveLawSetPoissonsRatio(myMatLin,0.2);

    //create section
    int mySection = myStructure.SectionCreate("Plane_Stress");
    myStructure.SectionSetThickness(mySection,1);

    //assign constitutive law
    myStructure.ElementTotalSetConstitutiveLaw(myMatLin);
    myStructure.ElementTotalSetSection(mySection);

    //set displacements of right node
    NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> directionX(2,1);
    directionX(0,0) = 1;
    NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> directionY(2,1);
    directionY(1,0) = 1;

    //left boundary
	int groupNodes_Left = myStructure.GroupCreate("Nodes");
	int direction=0;
	double min=0;
	double max=0;
	myStructure.GroupAddNodeCoordinateRange(groupNodes_Left,direction,min,max);

	//right boundary
	int groupNodes_Right = myStructure.GroupCreate("Nodes");
	direction=0;
	min=10;
	max=10;
	myStructure.GroupAddNodeCoordinateRange(groupNodes_Right,direction,min,max);

	int groupElements_Right = myStructure.GroupCreate("Elements");
	myStructure.GroupAddElementsFromNodes(groupElements_Right, groupNodes_Right);

    //define constant pressure at right hand side
    NuTo::FullVector<double, Eigen::Dynamic> loadVector(2);
	loadVector(0) = 1; //x-direction
	//myStructure.LoadSurfaceConstDirectionCreate2D(0, groupElements_Right, groupNodes_Right, loadVector);
	myStructure.LoadSurfacePressureCreate2D(0, groupElements_Right, groupNodes_Right, 1);

    //constraint for left hand side
	myStructure.ConstraintLinearSetDisplacementNode(0, directionY, 0.0);
	myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodes_Left,directionX,0);
	//myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodes_Right,directionX,1);

    //start analysis
    //build global dof numbering
    myStructure.NodeBuildGlobalDofs();

    //standard info
    myStructure.SetVerboseLevel(10);
    myStructure.Info();

    // build global stiffness matrix and equivalent load vector which correspond to prescribed boundary values
    NuTo::SparseMatrixCSRVector2General<double> stiffnessMatrixCSRVector2(0,0);
    NuTo::FullVector<double,Eigen::Dynamic> dispForceVector;
    myStructure.BuildGlobalCoefficientMatrix0(stiffnessMatrixCSRVector2, dispForceVector);
    NuTo::SparseMatrixCSRGeneral<double> stiffnessMatrix (stiffnessMatrixCSRVector2);

    // build global external load vector
    NuTo::FullVector<double,Eigen::Dynamic> extForceVector;
    myStructure.BuildGlobalExternalLoadVector(0,extForceVector);

    // calculate right hand side
    NuTo::FullVector<double,Eigen::Dynamic> rhsVector = dispForceVector + extForceVector;

    // solve
    NuTo::SparseDirectSolverMUMPS mySolver;
    NuTo::FullVector<double,Eigen::Dynamic> displacementVector;
    stiffnessMatrix.SetOneBasedIndexing();
    mySolver.Solve(stiffnessMatrix, rhsVector, displacementVector);

    // write displacements to node
    myStructure.NodeMergeActiveDofValues(displacementVector);

    //calculate engineering strain of myelement1 at all integration points
    //the size the matrix is not important and reallocated within the procedure
    NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> EngineeringStrain(6,1);
    //correct strain
    NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> EngineeringStrainCorrect(6,1);
    EngineeringStrainCorrect(0,0) = -0.1;
    EngineeringStrainCorrect(1,0) = 0.02;
    EngineeringStrainCorrect(2,0) = 0.02;

    NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> EngineeringStress(6,1);
    NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> EngineeringStressCorrect(6,1);
    EngineeringStressCorrect(0,0) = -1.00;

    bool error(false);
    for (int element=0; element<10; element++)
    {
		//calculate engineering strain at all integration points
		myStructure.ElementGetEngineeringStrain(element, EngineeringStrain);
		myStructure.ElementGetEngineeringStress(element, EngineeringStress);

		if (PRINTRESULT)
		{
			std::cout << "EngineeringStrainCorrect" << std::endl;
			EngineeringStrainCorrect.Info();
			std::cout << "EngineeringStrain" << std::endl;
			EngineeringStrain.Info();

			std::cout << "EngineeringStressCorrect" << std::endl;
			EngineeringStressCorrect.Info();
			std::cout << "EngineeringStress" << std::endl;
			EngineeringStress.Info();
		}

		for (int countIP=0; countIP<13; countIP++)
		{
			if ((EngineeringStrain.col(countIP)-EngineeringStrainCorrect).cwiseAbs().maxCoeff()>1e-8)
			{
				if (!PRINTRESULT)
				{
					std::cout << "EngineeringStrainCorrect" << std::endl;
					EngineeringStrainCorrect.Info();
					std::cout << "EngineeringStrain" << std::endl;
					EngineeringStrain.GetColumn(countIP).Info();
				}
				std::cout << "[Plane2D10N] : strain is not correct in element " <<  element << " and ip " << countIP << std::endl;
				error = true;
			}

			if ((EngineeringStress.col(countIP)-EngineeringStressCorrect).cwiseAbs().maxCoeff()>1e-8)
			{
				if (!PRINTRESULT)
				{
					std::cout << "EngineeringStressCorrect" << std::endl;
					EngineeringStressCorrect.Info();
					std::cout << "EngineeringStress" << std::endl;
					EngineeringStress.GetColumn(countIP).Info();
				}
				std::cout << "[Plane2D10N] : stress is not correct in element " <<  element << " and ip " << countIP << std::endl;
				error = true;
			}
		}
    }

    // visualize results
    myStructure.AddVisualizationComponentDisplacements();
    myStructure.AddVisualizationComponentEngineeringStrain();
    myStructure.AddVisualizationComponentEngineeringStress();
    myStructure.ExportVtkDataFileElements( "Plane2D10N_Elements.vtk");
    myStructure.ExportVtkDataFileNodes( "Plane2D10N_Nodes.vtk");

    if (error==true)
    {
    	return -1;
    }
}
catch (NuTo::Exception& e)
{
    std::cout << e.ErrorMessage() << std::endl;
    return -1;
}
return 0;
}
