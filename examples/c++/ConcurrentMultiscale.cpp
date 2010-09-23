#include "nuto/math/FullMatrix.h"
#include "nuto/mechanics/structures/unstructured/StructureIp.h"
#include "nuto/mechanics/MechanicsException.h"

#include "nuto/math/SparseDirectSolverMUMPS.h"
#include "nuto/math/SparseMatrixCSRVector2General.h"

#define MAXNUMNEWTONITERATIONS 20
//#define MAXNORMRHS 100
#define PRINTRESULT true
#define MIN_DELTA_STRAIN_FACTOR 1e-7

// there is still an error at the very end of the calculation
// uncomment at the end of void NuTo::NonlocalDamagePlasticity::YieldSurfaceRankine2DRoundedDerivatives
// the check for the derivatives of the yield surface to see the problem

int main()
{
try
{
    //
    double lX(100);
    double lY(100);

    //create structure
    NuTo::StructureIp myStructureFineScale(2);
    myStructureFineScale.SetShowTime(true);

    NuTo::FullMatrix<int> createdGroupIds;
    //myStructureFineScale.ImportFromGmsh("/home/unger3/develop/nuto/examples/c++/ConcurrentMultiscale.msh","displacements", "ConstitutiveLawIpNonlocal", "StaticDataNonlocal",createdGroupIds);
    myStructureFineScale.ImportFromGmsh("ConcurrentMultiscale.msh","displacements", "ConstitutiveLawIpNonlocal", "StaticDataNonlocal",createdGroupIds);

    //create constitutive law nonlocal damage
    int myMatDamage = myStructureFineScale.ConstitutiveLawCreate("NonlocalDamagePlasticity");
    double YoungsModulusDamage(20000);
    myStructureFineScale.ConstitutiveLawSetYoungsModulus(myMatDamage,YoungsModulusDamage);
    myStructureFineScale.ConstitutiveLawSetPoissonsRatio(myMatDamage,0.2);
    double nonlocalRadius(10);
    myStructureFineScale.ConstitutiveLawSetNonlocalRadius(myMatDamage,nonlocalRadius);
    double fct(2);
    myStructureFineScale.ConstitutiveLawSetTensileStrength(myMatDamage,fct);
    myStructureFineScale.ConstitutiveLawSetCompressiveStrength(myMatDamage,fct*10);
    myStructureFineScale.ConstitutiveLawSetBiaxialCompressiveStrength(myMatDamage,fct*12.5);
    myStructureFineScale.ConstitutiveLawSetFractureEnergy(myMatDamage,.1);

    //create constitutive law linear elastic (finally not used, since the elements are deleted)
    int myMatLinear = myStructureFineScale.ConstitutiveLawCreate("LinearElastic");
    double YoungsModulusLE(20000);
    myStructureFineScale.ConstitutiveLawSetYoungsModulus(myMatLinear,YoungsModulusLE);
    myStructureFineScale.ConstitutiveLawSetPoissonsRatio(myMatLinear,0.2);

    //create section
    double thickness(100);
    int mySectionParticle = myStructureFineScale.SectionCreate("Plane_Strain");
    myStructureFineScale.SectionSetThickness(mySectionParticle,thickness);

    int mySectionMatrix = myStructureFineScale.SectionCreate("Plane_Strain");
    myStructureFineScale.SectionSetThickness(mySectionMatrix,thickness);

    //assign constitutive law
//    myStructureFineScale.ElementGroupSetSection(101,mySectionParticle);
//    myStructureFineScale.ElementGroupSetSection(102,mySectionMatrix);
//    myStructureFineScale.ElementGroupSetConstitutiveLaw(101,myMatLinear);
//    myStructureFineScale.ElementGroupSetConstitutiveLaw(102,myMatDamage);
//    bool deleteNodes=true;
//    myStructureFineScale.ElementGroupDelete(101,deleteNodes);

    myStructureFineScale.ElementTotalSetSection(mySectionMatrix);
	myStructureFineScale.ElementTotalSetConstitutiveLaw(myMatDamage);

    //Build nonlocal elements
    myStructureFineScale.BuildNonlocalData(myMatDamage);

	//Create groups to apply the periodic boundary conditions
	//left boundary
	int GrpNodes_Left = myStructureFineScale.GroupCreate("Nodes");
	int direction = 0; //either 0,1,2
	double min(0.);
	double max(0.);
	myStructureFineScale.GroupAddNodeCoordinateRange(GrpNodes_Left,direction,min,max);

    //right boundary
    int GrpNodes_Right = myStructureFineScale.GroupCreate("Nodes");
    direction = 0; //either 0,1,2
    min=lX;
    max=lX;
    myStructureFineScale.GroupAddNodeCoordinateRange(GrpNodes_Right,direction,min,max);

    //top boundary
	int GrpNodes_Top = myStructureFineScale.GroupCreate("Nodes");
	direction=1;
	min=lY;
	max=lY;
	myStructureFineScale.GroupAddNodeCoordinateRange(GrpNodes_Top,direction,min,max);

    //bottom boundary
    int GrpNodes_Bottom = myStructureFineScale.GroupCreate("Nodes");
    direction=1;
    min=0;
    max=0;
    myStructureFineScale.GroupAddNodeCoordinateRange(GrpNodes_Bottom,direction,min,max);

	//top right node
    int GrpNodes_BottomLeftNode = myStructureFineScale.GroupIntersection(GrpNodes_Bottom,GrpNodes_Left);

#ifdef ENABLE_SERIALIZATION
    myStructureFineScale.Save("myStructureFineScale.xml","XML");
    myStructureFineScale.Restore("myStructureFineScale.xml","XML");
#endif // ENABLE_SERIALIZATION
    /*










    // applied strain and angle of boundary conditions
#define PI 3.14159265359
    //double angle(atan(1./3.)*180/PI);
    double angle(170);
    std::cout << "angle " <<  angle << std::endl;
    NuTo::FullMatrix<double> maxStrain(3,1);
    NuTo::FullMatrix<double> maxCrackOpening(2,1);
//    maxStrain(0,0) = 50.*fct/YoungsModulusDamage;
//    maxStrain(1,0) = 50.*fct/YoungsModulusDamage;
//    maxStrain(2,0) = -100*fct/YoungsModulusDamage;
    maxStrain(0,0) = 0.*fct/YoungsModulusDamage;
    maxStrain(1,0) = 0.*fct/YoungsModulusDamage;
    maxStrain(2,0) = 0.*fct/YoungsModulusDamage;
    maxCrackOpening(0,0) =  0.;
    maxCrackOpening(1,0) =  1;

    //fix bottom left corner node
	NuTo::FullMatrix<double> DirectionX(2,1);
	DirectionX.SetValue(0,0,1.0);
	DirectionX.SetValue(1,0,0.0);

	NuTo::FullMatrix<double>DirectionY(2,1);
	DirectionY.SetValue(0,0,0.0);
	DirectionY.SetValue(1,0,1.0);

    //myStructureFineScale.ConstraintSetDisplacementNodeGroup(GrpNodes_Bottom,DirectionY, 0);
    //myStructureFineScale.ConstraintSetDisplacementNodeGroup(GrpNodes_Left,DirectionX, 0);

    myStructureFineScale.ConstraintSetDisplacementNodeGroup(GrpNodes_BottomLeftNode,DirectionX, 0);
    myStructureFineScale.ConstraintSetDisplacementNodeGroup(GrpNodes_BottomLeftNode,DirectionY, 0);

#ifdef ENABLE_VISUALIZE
    myStructureFineScale.AddVisualizationComponentSection();
    myStructureFineScale.AddVisualizationComponentConstitutive();
    myStructureFineScale.AddVisualizationComponentDisplacements();
    myStructureFineScale.AddVisualizationComponentEngineeringStrain();
    myStructureFineScale.AddVisualizationComponentEngineeringStress();
    myStructureFineScale.AddVisualizationComponentDamage();
    myStructureFineScale.AddVisualizationComponentEngineeringPlasticStrain();
    myStructureFineScale.AddVisualizationComponentPrincipalEngineeringStress();
	myStructureFineScale.ElementTotalUpdateTmpStaticData();
    myStructureFineScale.ExportVtkDataFile("ConcurrentMultiscale.vtk");
#endif

    // init some result data
    NuTo::FullMatrix<double> PlotData(1,9);
    double externalEnergy(0.);

    // start analysis

    double deltaStrainFactor(0.0005);
    double maxDeltaStrainFactor(0.05);
    double curStrainFactor(0.00025);

    NuTo::FullMatrix<double> curStrain(maxStrain*curStrainFactor);
    NuTo::FullMatrix<double> curCrackOpening(maxCrackOpening*curStrainFactor);

    //update displacement of boundary (disp controlled)
    double radiusToCrackWithoutConstraints(nonlocalRadius*0.0);
    int constraintPeriodic = myStructureFineScale.ConstraintDisplacementsSetPeriodic2D(angle, curStrain,
            curCrackOpening, radiusToCrackWithoutConstraints, GrpNodes_Top, GrpNodes_Bottom, GrpNodes_Left,GrpNodes_Right);

	//update conre mat
	myStructureFineScale.NodeBuildGlobalDofs();

	//update tmpstatic data with zero displacements
	myStructureFineScale.ElementTotalUpdateTmpStaticData();

	//init some auxiliary variables
	NuTo::SparseMatrixCSRVector2General<double> stiffnessMatrixCSRVector2;
	NuTo::FullMatrix<double> dispForceVector;
	NuTo::FullMatrix<double> intForceVector;
	NuTo::FullMatrix<double> extForceVector;
	NuTo::FullMatrix<double> rhsVector;

	//allocate solver
	NuTo::SparseDirectSolverMUMPS mySolver;
	mySolver.SetShowTime(true);

    //calculate stiffness
	myStructureFineScale.BuildGlobalCoefficientMatrix0(stiffnessMatrixCSRVector2, dispForceVector);

	// build global external load vector and RHS vector
	myStructureFineScale.BuildGlobalExternalLoadVector(extForceVector);
	rhsVector = extForceVector + dispForceVector;

	//calculate absolute tolerance for matrix entries to be not considered as zero
	double maxValue, minValue, ToleranceZeroStiffness;
	stiffnessMatrixCSRVector2.Max(maxValue);
	stiffnessMatrixCSRVector2.Min(minValue);
	std::cout << "min and max " << minValue << " , " << maxValue << std::endl;

	ToleranceZeroStiffness = (1e-14) * (fabs(maxValue)>fabs(minValue) ?  fabs(maxValue) : fabs(minValue));
	myStructureFineScale.SetToleranceStiffnessEntries(ToleranceZeroStiffness);
	int numRemoved = stiffnessMatrixCSRVector2.RemoveZeroEntries(ToleranceZeroStiffness,0);
	int numEntries = stiffnessMatrixCSRVector2.GetNumEntries();
	std::cout << "stiffnessMatrix: num zero removed " << numRemoved << ", numEntries " << numEntries << std::endl;

	//update displacements of all nodes according to the new conre mat
	{
	    NuTo::FullMatrix<double> displacementsActiveDOFsCheck;
	    NuTo::FullMatrix<double> displacementsDependentDOFsCheck;
	    myStructureFineScale.NodeExtractDofValues(displacementsActiveDOFsCheck, displacementsDependentDOFsCheck);
	    myStructureFineScale.NodeMergeActiveDofValues(displacementsActiveDOFsCheck);
	    myStructureFineScale.ElementTotalUpdateTmpStaticData();
	}

	//repeat until max displacement is reached
	bool convergenceStatusLoadSteps(false);
	while (!convergenceStatusLoadSteps)
    {

        double normResidual(1);
        double maxResidual(1);
        int numNewtonIterations(0);
		double normRHS(1.);
		double alpha(1.);
		int convergenceStatus(0);
		//0 - not converged, continue Newton iteration
		//1 - converged
		//2 - stop iteration, decrease load step
		while(convergenceStatus==0)
		{
			numNewtonIterations++;

			if (numNewtonIterations>MAXNUMNEWTONITERATIONS)
			{
				if (PRINTRESULT)
		    	{
				    std::cout << "numNewtonIterations (" << numNewtonIterations << ") > MAXNUMNEWTONITERATIONS (" << MAXNUMNEWTONITERATIONS << ")" << std::endl;
		    	}
				convergenceStatus = 2; //decrease load step
				break;
			}

			normRHS = rhsVector.Norm();

			// solve
			NuTo::FullMatrix<double> deltaDisplacementsActiveDOFs;
			NuTo::FullMatrix<double> oldDisplacementsActiveDOFs;
			NuTo::FullMatrix<double> displacementsActiveDOFs;
			NuTo::FullMatrix<double> displacementsDependentDOFs;
			NuTo::SparseMatrixCSRGeneral<double> stiffnessMatrixCSR(stiffnessMatrixCSRVector2);
			stiffnessMatrixCSR.SetOneBasedIndexing();
			mySolver.Solve(stiffnessMatrixCSR, rhsVector, deltaDisplacementsActiveDOFs);

			// write displacements to node
			myStructureFineScale.NodeExtractDofValues(oldDisplacementsActiveDOFs, displacementsDependentDOFs);

			//perform a linesearch
			alpha = 1.;
			do
			{
				//add new displacement state
				displacementsActiveDOFs = oldDisplacementsActiveDOFs + deltaDisplacementsActiveDOFs*alpha;
				myStructureFineScale.NodeMergeActiveDofValues(displacementsActiveDOFs);
				myStructureFineScale.ElementTotalUpdateTmpStaticData();

				// calculate residual
				myStructureFineScale.BuildGlobalGradientInternalPotentialVector(intForceVector);
				rhsVector = extForceVector - intForceVector;
				normResidual = rhsVector.Norm();
				std::cout << "alpha " << alpha << ", normResidual " << normResidual << ", normResidualInit "<< normRHS << ", normRHS*(1-0.5*alpha) " << normRHS*(1-0.5*alpha) << std::endl;
				alpha*=0.5;
			}
			while(alpha>1e-3 && normResidual>normRHS*(1-0.5*alpha) && normResidual>1e-5);
			if (normResidual>normRHS*(1-0.5*alpha) && normResidual>1e-5)
			{
			    convergenceStatus=2;
			    break;
			}

		    maxResidual = rhsVector.Max();

			std::cout << std::endl << "Newton iteration " << numNewtonIterations << ", final alpha " << 2*alpha << ", normResidual " << normResidual<< ", maxResidual " << maxResidual<<std::endl;
			//char cDummy[100]="";
			//std::cin.getline(cDummy, 100);

			//check convergence
			if (normResidual<1e-5 || maxResidual<1e-5)
			{
				if (PRINTRESULT)
		    	{
					std::cout << "Convergence after " << numNewtonIterations << " Newton iterations, curStrainFactor " << curStrainFactor << ", deltaStrainFactor "<< deltaStrainFactor << std::endl<< std::endl;
		    	}
				convergenceStatus=1;
				break;
			}

			//convergence status == 0 (continue Newton iteration)
			//build new stiffness matrix
			myStructureFineScale.BuildGlobalCoefficientMatrix0(stiffnessMatrixCSRVector2, dispForceVector);
			int numRemoved = stiffnessMatrixCSRVector2.RemoveZeroEntries(ToleranceZeroStiffness,0);
			int numEntries = stiffnessMatrixCSRVector2.GetNumEntries();
			std::cout << "stiffnessMatrix: num zero removed " << numRemoved << ", numEntries " << numEntries << std::endl;
		}

		if (deltaStrainFactor==0)
		    throw NuTo::MechanicsException("Example ConcurrentMultiscale : No convergence, delta strain factor < 1e-7");

		if (convergenceStatus==1)
		{
			myStructureFineScale.ElementTotalUpdateStaticData();

            // visualize results
#ifdef ENABLE_VISUALIZE
            myStructureFineScale.ExportVtkDataFile("ConcurrentMultiscale.vtk");
#endif
 			//store result/plot data
			NuTo::FullMatrix<double> SinglePlotData(1,9);
			SinglePlotData(0,0) = curStrain(0,0);
            SinglePlotData(0,1) = curStrain(1,0);
            SinglePlotData(0,2) = curStrain(2,0);
            //Get Average Stress
            NuTo::FullMatrix<double> averageStress;
	        myStructureFineScale.ElementTotalGetAverageStress(lX*lY,averageStress);
            SinglePlotData(0,3) = averageStress(0,0);
            SinglePlotData(0,4) = averageStress(1,0);
            SinglePlotData(0,5) = averageStress(2,0);
            SinglePlotData(0,6) = averageStress(3,0);
            SinglePlotData(0,7) = averageStress(4,0);
            SinglePlotData(0,8) = averageStress(5,0);

            std::cout << "average stress " << std::endl << averageStress << std::endl;
            std::cout << "current Strain " << std::endl << curStrain << std::endl;

			externalEnergy+=averageStress(0,0)*curStrain(0,0)+averageStress(1,0)*curStrain(1,0)+averageStress(2,0)*curStrain(2,0);

			PlotData.AppendRows(SinglePlotData);
		    PlotData.WriteToFile("ConcurrentMultiscaleLoadDisp.txt"," ","#load displacement curve, disp, stress, force, sxx in center element, syy in center element","  ");

            if (curStrainFactor==1)
                convergenceStatusLoadSteps=true;
            else
            {
                //eventually increase load step
                if (numNewtonIterations<MAXNUMNEWTONITERATIONS/3)
                {
                    deltaStrainFactor*=1.5;
                }

                //increase displacement
                curStrainFactor+=deltaStrainFactor;
                if (curStrainFactor>1)
                {
                    deltaStrainFactor -= curStrainFactor -1.;
                    curStrainFactor=1;
                }

                curStrain = maxStrain*curStrainFactor;
                curCrackOpening = maxCrackOpening*curStrainFactor;


                //old stiffness matrix is used in first step of next load increment in order to prevent spurious problems at the boundary
                std::cout << "press enter to next load increment, delta strain factor " << deltaStrainFactor << " max delta strain factor " <<  maxDeltaStrainFactor << std::endl << std::endl;
                //char cDummy[100]="";
                //std::cin.getline(cDummy, 100);
            }
        }
		else
		{
            assert(convergenceStatus==2);
			//calculate stiffness of previous loadstep (used as initial stiffness in the next load step)
			//this is done within the loop in order to ensure, that for the first step the stiffness matrix of the previous step is used
			//otherwise, the additional boundary displacements will result in an artifical localization in elements at the boundary
			curStrainFactor-=deltaStrainFactor;
            curStrain = maxStrain*curStrainFactor;
            curCrackOpening = maxCrackOpening*curStrainFactor;

			myStructureFineScale.ConstraintPeriodicSetStrain(constraintPeriodic,curStrain);
            myStructureFineScale.ConstraintPeriodicSetCrackOpening(constraintPeriodic,curCrackOpening);

			// build global dof numbering
			myStructureFineScale.NodeBuildGlobalDofs();

			//set previous converged displacements
			NuTo::FullMatrix<double> displacementsActiveDOFsCheck;
			NuTo::FullMatrix<double> displacementsDependentDOFsCheck;
			myStructureFineScale.NodeExtractDofValues(displacementsActiveDOFsCheck, displacementsDependentDOFsCheck);
			myStructureFineScale.NodeMergeActiveDofValues(displacementsActiveDOFsCheck);
			myStructureFineScale.ElementTotalUpdateTmpStaticData();

			// calculate previous residual (should be almost zero)
			myStructureFineScale.BuildGlobalGradientInternalPotentialVector(intForceVector);

			//decrease load step
			deltaStrainFactor*=0.5;
			curStrainFactor+=deltaStrainFactor;

			//check for minimum delta (this mostly indicates an error in the software
			if (deltaStrainFactor<MIN_DELTA_STRAIN_FACTOR)
			{
			    deltaStrainFactor = 0;
			    //throw NuTo::MechanicsException("Example ConcurrentMultiscale : No convergence, delta strain factor < 1e-7");
			}

			std::cout << "press enter to reduce load increment" << std::endl;
			//char cDummy[100]="";
			//std::cin.getline(cDummy, 100);;
		}

		if (!convergenceStatusLoadSteps)
		{
            //update new displacement of RHS
            myStructureFineScale.ConstraintPeriodicSetStrain(constraintPeriodic,curStrain);
            myStructureFineScale.ConstraintPeriodicSetCrackOpening(constraintPeriodic,curCrackOpening);

            // build global dof numbering
            myStructureFineScale.NodeBuildGlobalDofs();

            //update stiffness in order to calculate new dispForceVector
            myStructureFineScale.BuildGlobalCoefficientMatrix0(stiffnessMatrixCSRVector2, dispForceVector);
            int numRemoved = stiffnessMatrixCSRVector2.RemoveZeroEntries(ToleranceZeroStiffness,0);
            int numEntries = stiffnessMatrixCSRVector2.GetNumEntries();
            std::cout << "stiffnessMatrix: num zero removed " << numRemoved << ", numEntries " << numEntries << std::endl;

            //update rhs vector for next Newton iteration
            rhsVector = dispForceVector + extForceVector - intForceVector;

            //update displacements of all nodes according to the new conre mat
            NuTo::FullMatrix<double> displacementsActiveDOFsCheck;
            NuTo::FullMatrix<double> displacementsDependentDOFsCheck;
            myStructureFineScale.NodeExtractDofValues(displacementsActiveDOFsCheck, displacementsDependentDOFsCheck);
            myStructureFineScale.NodeMergeActiveDofValues(displacementsActiveDOFsCheck);
            myStructureFineScale.ElementTotalUpdateTmpStaticData();
		}
    }
	if (PRINTRESULT)
	{
        std::cout<< "numerical fracture energy "<< externalEnergy/(thickness*lY) << std::endl;
	}
*/
}
catch (NuTo::Exception& e)
{
    std::cout << e.ErrorMessage() << std::endl;
}
return 0;
}