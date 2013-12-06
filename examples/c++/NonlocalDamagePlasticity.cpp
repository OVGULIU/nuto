#include "nuto/math/FullMatrix.h"
#include "nuto/math/FullVector.h"
#include "nuto/mechanics/structures/unstructured/Structure.h"

//just for test
#include "nuto/mechanics/constitutive/mechanics/DeformationGradient2D.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStrain2D.h"
#include "nuto/mechanics/constitutive/mechanics/NonlocalDamagePlasticityEngineeringStress.h"
#include <eigen3/Eigen/Core>

#define printResult true

int main()
{
    try
    {
	//create structure
	NuTo::Structure myStructure(2);

	//3x3 nodes 2x2 element grid
	//create nodes
	NuTo::FullVector<double,Eigen::Dynamic> Coordinates(2);
	Coordinates(0) = 0.0;
	Coordinates(1) = 0.0;
	int node1 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0) = 1.0;
	Coordinates(1) = 0.0;
	int node2 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0) = 2.0;
	Coordinates(1) = 0.0;
	int node3 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0) = 0.0;
	Coordinates(1) = 1.0;
	int node4 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0) = 1.0;
	Coordinates(1) = 1.0;
	int node5 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0) = 2.0;
	Coordinates(1) = 1.0;
	int node6 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0) = 0.0;
	Coordinates(1,0) = 2.0;
	int node7 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0) = 1.0;
	Coordinates(1) = 2.0;
	int node8 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0) = 2.0;
	Coordinates(1) = 2.0;
	int node9 = myStructure.NodeCreate("displacements",Coordinates);

	//create elements
	NuTo::FullVector<int,Eigen::Dynamic> Incidence(4);
	Incidence(0) = node1;
	Incidence(1) = node2;
	Incidence(2) = node5;
	Incidence(3) = node4;
    int myElement1 = myStructure.ElementCreate("PLANE2D4N",Incidence,"ConstitutiveLawIpNonlocal","StaticDataNonlocal");
    myStructure.ElementSetIntegrationType(myElement1,"2D4NGauss1Ip","StaticDataNonlocal");

	Incidence(0) = node2;
	Incidence(1) = node3;
	Incidence(2) = node6;
	Incidence(3) = node5;
    int myElement2 = myStructure.ElementCreate("PLANE2D4N",Incidence,"ConstitutiveLawIpNonlocal","StaticDataNonlocal");
    myStructure.ElementSetIntegrationType(myElement2,"2D4NGauss4Ip","StaticDataNonlocal");
		
	Incidence(0) = node4;
	Incidence(1) = node5;
	Incidence(2) = node8;
	Incidence(3) = node7;
    int myElement3 = myStructure.ElementCreate("PLANE2D4N",Incidence,"ConstitutiveLawIpNonlocal","StaticDataNonlocal");
    myStructure.ElementSetIntegrationType(myElement3,"2D4NGauss1Ip","StaticDataNonlocal");
		
	Incidence(0) = node5;
	Incidence(1) = node6;
	Incidence(2) = node9;
	Incidence(3) = node8;
    int myElement4 = myStructure.ElementCreate("PLANE2D4N",Incidence,"ConstitutiveLawIpNonlocal","StaticDataNonlocal");
    myStructure.ElementSetIntegrationType(myElement4,"2D4NGauss4Ip","StaticDataNonlocal");

	//create constitutive law
	int myMatDamage = myStructure.ConstitutiveLawCreate("NonlocalDamagePlasticityEngineeringStress");
	myStructure.ConstitutiveLawSetYoungsModulus(myMatDamage,9);
	myStructure.ConstitutiveLawSetPoissonsRatio(myMatDamage,0.25);
	myStructure.ConstitutiveLawSetNonlocalRadius(myMatDamage,0.7);
	myStructure.ConstitutiveLawSetTensileStrength(myMatDamage,2);
	myStructure.ConstitutiveLawSetCompressiveStrength(myMatDamage,20);
	myStructure.ConstitutiveLawSetBiaxialCompressiveStrength(myMatDamage,25);
	myStructure.ConstitutiveLawSetFractureEnergy(myMatDamage,0.2);

	int myMatLin = myStructure.ConstitutiveLawCreate("LinearElasticEngineeringStress");
	myStructure.ConstitutiveLawSetYoungsModulus(myMatLin,9);
	myStructure.ConstitutiveLawSetPoissonsRatio(myMatLin,0.25);

	//create section
	int mySection = myStructure.SectionCreate("Plane_Strain");
	myStructure.SectionSetThickness(mySection,0.5);

	//assign constitutive law 
	myStructure.ElementTotalSetSection(mySection);
	myStructure.ElementTotalSetConstitutiveLaw(myMatDamage);

	//Build nonlocal elements
	myStructure.BuildNonlocalData(myMatDamage);

	//Calculate maximum independent sets for parallelization (openmp)
    //myStructure.CalculateMaximumIndependentSets();

	// visualize results
	myStructure.AddVisualizationComponentNonlocalWeights(myElement1,0);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement2,0);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement2,1);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement2,2);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement2,3);
	myStructure.ElementTotalUpdateTmpStaticData();
	//myStructure.ExportVtkDataFileElements("PlaneNonlocalWeights.vtk");

	NuTo::SparseMatrixCSRVector2General<double> stiffnessMatrix(0,0);
	NuTo::FullVector<double,Eigen::Dynamic> dispForceVector(0);

	myStructure.ElementTotalUpdateTmpStaticData();
	myStructure.BuildGlobalCoefficientMatrix0(stiffnessMatrix, dispForceVector);
	stiffnessMatrix.RemoveZeroEntries(0,1e-14);

	NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> fullStiffnessMatrixElastic(stiffnessMatrix);
	if (printResult)
	{
	    std::cout << "stiffnessMatrix elastic" << std::endl;
	    fullStiffnessMatrixElastic.Info();
	}

	NuTo::FullVector<double,Eigen::Dynamic> displacements,dependentDofs,intForce,intForce2;


//check the stiffness three times
//loadstep 0 : uniform plastic loading
//loadstep 1 : unloading to zero
//loadstep 2 : nonuniform loading, some elements unloading
	double rightDisp;
	for (int theLoadStep=0; theLoadStep<2; theLoadStep++)
	{
		//apply displacements
		if (theLoadStep==0)
			rightDisp = 0.5;
		else if (theLoadStep==1)
			rightDisp = 0.0;
		else
			rightDisp = 0.6;

		NuTo::FullVector<double,Eigen::Dynamic>  matrixRightDisp(2);
		matrixRightDisp.SetValue(0,0,rightDisp);
		matrixRightDisp.SetValue(1,0,0.);

		myStructure.NodeSetDisplacements(node3,matrixRightDisp);
		myStructure.NodeSetDisplacements(node6,matrixRightDisp);
		myStructure.NodeSetDisplacements(node9,matrixRightDisp);

		NuTo::FullVector<double,Eigen::Dynamic>  matrixCenterDisp(2);
		if (theLoadStep!=2)
			matrixCenterDisp.SetValue(0,0,0.5*rightDisp);
		else
			matrixCenterDisp.SetValue(0,0,0.4*rightDisp);
		matrixCenterDisp.SetValue(1,0,0.);

		myStructure.NodeSetDisplacements(node2,matrixCenterDisp);
		myStructure.NodeSetDisplacements(node5,matrixCenterDisp);
		myStructure.NodeSetDisplacements(node8,matrixCenterDisp);

		NuTo::FullVector<double,Eigen::Dynamic>  matrixLeftDisp(2);
		matrixLeftDisp.SetValue(0,0,0.0);
		matrixLeftDisp.SetValue(1,0,0.);

		myStructure.NodeSetDisplacements(node1,matrixLeftDisp);
		myStructure.NodeSetDisplacements(node4,matrixLeftDisp);
		myStructure.NodeSetDisplacements(node7,matrixLeftDisp);

		myStructure.ElementTotalUpdateTmpStaticData();

		myStructure.BuildGlobalCoefficientMatrix0(stiffnessMatrix, dispForceVector);
		stiffnessMatrix.RemoveZeroEntries(0,1e-14);

		NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> fullStiffnessMatrix (stiffnessMatrix);

		if (printResult)
		{
			std::cout << "fullStiffnessMatrix analytic" << std::endl;
			fullStiffnessMatrix.Info();
		}

		myStructure.NodeExtractDofValues(displacements,dependentDofs);
		myStructure.BuildGlobalGradientInternalPotentialVector(intForce);

		double delta = 1e-8;
		NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> stiffnessMatrixCD(displacements.GetNumRows(),displacements.GetNumRows());

		//check with central differences
		for (int count2=0; count2<displacements.GetNumRows();count2++)
		{
			displacements.AddValue(count2,0,delta);
			myStructure.NodeMergeActiveDofValues(displacements);
			myStructure.ElementTotalUpdateTmpStaticData();
			myStructure.BuildGlobalGradientInternalPotentialVector(intForce2);
			stiffnessMatrixCD.SetColumn(count2,(intForce2-intForce)*(1./delta));
			displacements.AddValue(count2,0,-delta);
		}

		if (printResult)
		{
			std::cout <<  "fullStiffnessMatrix" << std::endl;
			fullStiffnessMatrix.Info();
			std::cout << "stiffness CD" << std::endl;
			stiffnessMatrixCD.Info();
		}

		double maxerror=(fullStiffnessMatrix-stiffnessMatrixCD).cwiseAbs().maxCoeff();

		bool error(false);
		if (theLoadStep==0)
		{
			if (printResult)
				std::cout << "max difference in stiffness matrix for uniform plastic loading " <<  maxerror << std::endl;
			if (maxerror>1e-6)
			{
				std::cout << "stiffness for uniform plastic loading is not correct." << std::endl;
				error = true;
			}
		}
		else if (theLoadStep==1)
		{
			if (printResult)
				std::cout << "max difference in stiffness matrix for unloading " << maxerror << std::endl;
			if (maxerror>1e-6)
			{
				std::cout << "stiffness after unloading is not correct." << std::endl;
				error = true;
			}
			double omega = fullStiffnessMatrix.GetValue(0,0)/fullStiffnessMatrixElastic.GetValue(0,0);
			double maxerror2 = (fullStiffnessMatrix - fullStiffnessMatrixElastic*omega).cwiseAbs().maxCoeff();
			if (printResult)
			{
				std::cout << "difference in stiffness matrix for unloading and scaled elastic matrix" << std::endl;
				NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> diffMatrix(fullStiffnessMatrixElastic*omega-fullStiffnessMatrix);
				diffMatrix.Info();
			}
			if (printResult)
				std::cout << "max difference in stiffness matrix for unloading and scaled elastic matrix " << maxerror2 << std::endl;
			if (maxerror2>1e-6)
				std::cout << "stiffness matrix for unloading and scaled elastic matrix are not identical." << std::endl;
				error = true;
		}
		else
		{
			if (printResult)
				std::cout << "max difference in stiffness matrix for nonuniform plastic loading/unloading " << maxerror << std::endl;
			if (maxerror>1e-6)
				std::cout << "stiffness matrix for nonuniform plastic loading/unloading is not correct." << std::endl;
				error = true;
		}

		//update the structure, and then recalculate stiffness
		myStructure.ElementTotalUpdateStaticData();

		if (error)
		{
			throw NuTo::Exception("Error calling NonlocalDamagePlasticity.");
		}
    }

	myStructure.AddVisualizationComponentDisplacements();
	myStructure.AddVisualizationComponentEngineeringStrain();
	myStructure.AddVisualizationComponentEngineeringStress();
	myStructure.AddVisualizationComponentDamage();
	myStructure.AddVisualizationComponentEngineeringPlasticStrain();
	myStructure.ExportVtkDataFileElements("NonlocalDamagePlasticityModel.vtk");

    }
    catch (NuTo::Exception& e)
    {
        std::cout << e.ErrorMessage() << std::endl;
    }
    return 0;
}
