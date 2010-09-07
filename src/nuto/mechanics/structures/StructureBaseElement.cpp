#ifdef SHOW_TIME
    #include <ctime>
#endif
#include <assert.h>
#include <boost/tokenizer.hpp>
#include "nuto/mechanics/structures/StructureBase.h"
#include "nuto/mechanics/groups/Group.h"
#include "nuto/mechanics/elements/Truss1D2N.h"
#include "nuto/mechanics/nodes/NodeBase.h"

//! @brief calls ElementCoefficientMatrix_0,
//! renaming only for clarification in mechanical problems for the end user
void NuTo::StructureBase::ElementStiffness(int rElementId, NuTo::FullMatrix<double>& rResult ,
		NuTo::FullMatrix<int>& rGlobalDofsRow,
		NuTo::FullMatrix<int>& rGlobalDofsColumn)const
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
	try
	{
		ElementCoefficientMatrix_0(rElementId, rResult, rGlobalDofsRow, rGlobalDofsColumn);
	}
	catch(NuTo::MechanicsException &e)
	{
		std::stringstream ss;
		ss << rElementId;
		std::string s = ss.str(); //Gets you a C++ STL string
		e.AddMessage("[NuTo::StructureBase::ElementStiffness] Error calculating stiffness of element "
				+ s + ".");
		throw e;
	}
	catch (...)
	{
		std::stringstream ss;
		ss << rElementId;
		std::string s = ss.str(); //Gets you a C++ STL string
		throw MechanicsException("[NuTo::StructureBase::ElementStiffness] Error calculating stiffness of element "
				+ s + ".");
	}
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementStiffness] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief calculates the coefficient matrix for the 0-th derivative in the differential equation
//! and compares it to the matrix using central differences
//! for a mechanical problem, this corresponds to the stiffness matrix
//! @param rDelta  delta step for finite differences
//! @return element with maximum error
int NuTo::StructureBase::ElementTotalCoefficientMatrix_0_Check(double rDelta, NuTo::FullMatrix<double>& rDifference)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif

    std::vector<ElementBase*> elementVector;
    GetElementsTotal(elementVector);
    NuTo::FullMatrix<double> tmpDifference;
    double maxError(0);
    int maxElement(-1);
	std::vector<int> globalDofsRow,globalDofsColumn;
	NuTo::FullMatrix<double> stiffnessAnalytic;
	NuTo::FullMatrix<double> stiffnessCDF;
	bool symmetryFlag;
    for (unsigned int countElement=0;  countElement<elementVector.size();countElement++)
    {
        try
        {
        	//std::cout << "Element Id " << this->ElementGetId(elementVector[countElement]) << std::endl << std::endl;

        	elementVector[countElement]->CalculateCoefficientMatrix_0(stiffnessAnalytic, globalDofsRow, globalDofsColumn, symmetryFlag);
        	//std::cout << "stiffnessAnalytic " << std::endl << stiffnessAnalytic << std::endl << std::endl;

        	stiffnessCDF.Resize(stiffnessAnalytic.GetNumRows(),stiffnessAnalytic.GetNumColumns());
        	this->ElementCoefficientMatrix_0_Resforce(elementVector[countElement],rDelta,stiffnessCDF);
        	//std::cout << "stiffnessCDF " << std::endl << stiffnessCDF << std::endl << std::endl;

        	//check the maximum error
        	tmpDifference = (stiffnessCDF-stiffnessAnalytic)*(1./stiffnessAnalytic.Abs().Max());
        	//std::cout << "difference "<< std::endl << tmpDifference <<std::endl << std::endl;
        	double curError =  tmpDifference.Abs().Max();
        	if (curError>maxError)
        	{
        		maxError = curError;
        		maxElement = countElement;
        		rDifference = tmpDifference;
        	}
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[countElement]);
            e.AddMessage("[NuTo::StructureBase::ElementTotalCoefficientMatrix_0_Check] Error checking stiffness for "
            		+ ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[countElement]);
        	throw NuTo::MechanicsException
        	   ("[NuTo::StructureBase::ElementTotalCoefficientMatrix_0_Check] Error checking stiffness for "
        			   + ss.str() + ".");
        }
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementTotalCoefficientMatrix_0_Check] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
    return this->ElementGetId(elementVector[maxElement]);
}

//! @brief calculates the coefficient matrix for the 0-th derivative in the differential equation
//! and compares it to the matrix using central differences
//! for a mechanical problem, this corresponds to the stiffness matrix
//! @param rElementId element
//! @param rDelta  delta step for finite differences
//! @return maximum difference between analytical and central difference method
double NuTo::StructureBase::ElementCoefficientMatrix_0_Check(int rElementId, double rDelta, NuTo::FullMatrix<double>& rDifference)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    // build global tmp static data
    if (this->mHaveTmpStaticData && this->mUpdateTmpStaticDataRequired)
    {
        throw MechanicsException("[NuTo::StructureBase::ElementCoefficientMatrix_0_Check] First update of tmp static data required.");
    }

    ElementBase* elementPtr = ElementGetElementPtr(rElementId);
    double maxError;

    try
    {
    	std::vector<int> globalDofsRow,globalDofsColumn;
    	NuTo::FullMatrix<double> stiffnessAnalytic;
    	NuTo::FullMatrix<double> stiffnessCDF;
    	bool symmetryFlag;
    	elementPtr->CalculateCoefficientMatrix_0(stiffnessAnalytic, globalDofsRow, globalDofsColumn, symmetryFlag);
    	//std::cout << "stiffnessAnalytic " << std::endl << stiffnessAnalytic << std::endl << std::endl;

    	stiffnessCDF.Resize(stiffnessAnalytic.GetNumRows(),stiffnessAnalytic.GetNumColumns());
    	this->ElementCoefficientMatrix_0_Resforce(elementPtr,rDelta,stiffnessCDF);
    	//std::cout << "stiffnessCDF " << std::endl << stiffnessCDF << std::endl << std::endl;

    	//check the maximum error
    	rDifference = (stiffnessCDF-stiffnessAnalytic)*(1./stiffnessAnalytic.Abs().Max());
    	//std::cout << "rDifference "<< std::endl << rDifference <<std::endl << std::endl;
        maxError =  rDifference.Abs().Max();
    }
    catch(NuTo::MechanicsException e)
    {
        std::stringstream ss;
        ss << rElementId;
    	e.AddMessage("[NuTo::StructureBase::ElementCoefficientMatrix_0_Check] Error checking element matrix for element "
        	+ ss.str() + ".");
        throw e;
    }
    catch(...)
    {
        std::stringstream ss;
        ss << rElementId;
    	throw NuTo::MechanicsException
    	   ("[NuTo::StructureBase::ElementCoefficientMatrix_0_Check] Error checking element matrix for element " + ss.str() + ".");
    }

#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementCoefficientMatrix_0_Check] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
    return maxError;
}

//! @brief calculates the coefficient matrix for the 0-th derivative in the differential equation
//! @param rElementId elementId
//! @param rDelta  delta step for finite differences
//! @param stiffnessCDF  stiffness from central differences (return value, size should be allocated correctly before entering the routine)
//! @return maximum difference between analytical and central difference method
void NuTo::StructureBase::ElementCoefficientMatrix_0_Resforce(int rElementId, double rDelta, NuTo::FullMatrix<double>& stiffnessCDF)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    ElementBase* elementPtr = ElementGetElementPtr(rElementId);
    ElementCoefficientMatrix_0_Resforce(elementPtr,rDelta,stiffnessCDF);

#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementCoefficientMatrix_0_Resforce] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief calculates the coefficient matrix for the 0-th derivative in the differential equation
//! @param rElementPtr element
//! @param rDelta  delta step for finite differences
//! @param stiffnessCDF  stiffness from central differences (return value, size should be allocated correctly before entering the routine)
//! @return maximum difference between analytical and central difference method
void NuTo::StructureBase::ElementCoefficientMatrix_0_Resforce(ElementBase* rElementPtr, double rDelta, NuTo::FullMatrix<double>& stiffnessCDF)
{
	NuTo::FullMatrix<double> resforce1;
	NuTo::FullMatrix<double> resforce2;

	// if other DOFs than displacements are implemented, a routine should be implemented for all elements, that
	// specifies wich DOFs are used

	int numNonlocalElements = rElementPtr->GetNumNonlocalElements();
	std::vector<const NuTo::ElementBase*>  nonlocalElements;
	if (numNonlocalElements==0 || stiffnessCDF.GetNumRows()==stiffnessCDF.GetNumColumns())
	{
		//local formulation
		nonlocalElements.push_back(rElementPtr);
	}
	else
	{
		nonlocalElements = rElementPtr->GetNonlocalElements();
	}

	//determine the initial trial resforce vector
	// if stiffnessAnalytic is square with nonlocal!=0, then all ips are elastic
	std::vector<int> globalDofsRow;
	//update tmpstatic data of all nonlocal elements
	if (mHaveTmpStaticData)
	{
		for (unsigned int countElement2=0; countElement2<nonlocalElements.size(); countElement2++)
		{
            const_cast<NuTo::ElementBase*>(nonlocalElements[countElement2])->UpdateStaticData(NuTo::Element::TMPSTATICDATA);
		}
	}
	rElementPtr->CalculateGradientInternalPotential(resforce1,globalDofsRow);

	int curCol(0);

	for (unsigned int countElement=0; countElement<nonlocalElements.size(); countElement++)
	{
		for (int countNode=0; countNode<nonlocalElements[countElement]->GetNumNodes(); countNode++)
		{
			NodeBase *theNode=const_cast<NuTo::ElementBase*>(nonlocalElements[countElement])->GetNode(countNode);
			double disp[3];
			assert(theNode->GetNumDisplacements()<=3);
			for (int countDisp=0; countDisp<theNode->GetNumDisplacements(); countDisp++)
			{
				switch (theNode->GetNumDisplacements())
				{
				case 1:
					theNode->GetDisplacements1D(disp);
					break;
				case 2:
					theNode->GetDisplacements2D(disp);
					break;
				case 3:
					theNode->GetDisplacements3D(disp);
					break;
				default:
					throw MechanicsException("[NuTo::StructureBase::ElementCoefficientMatrix_0_Check] Only nodes with 1,2 or 3 displacement components considered.");
				}
				disp[countDisp]+=rDelta;
				switch (theNode->GetNumDisplacements())
				{
				case 1:
					theNode->SetDisplacements1D(disp);
					break;
				case 2:
					theNode->SetDisplacements2D(disp);
					break;
				case 3:
					theNode->SetDisplacements3D(disp);
					break;
				default:
					throw MechanicsException("[NuTo::StructureBase::ElementCoefficientMatrix_0_Check] Only nodes with 1,2 or 3 displacement components considered.");
				}

				//update tmpstatic data of nonlocal elements
				if (mHaveTmpStaticData)
				{
                     const_cast<NuTo::ElementBase*>(nonlocalElements[countElement])->UpdateStaticData(NuTo::Element::TMPSTATICDATA);
				}

				if (nonlocalElements[countElement]==rElementPtr)
				{
					//calculate new residual vector and afterwards reset the displacements
					rElementPtr->CalculateGradientInternalPotential(resforce2, globalDofsRow);

					disp[countDisp]-=rDelta;
					switch (theNode->GetNumDisplacements())
					{
					case 1:
						theNode->SetDisplacements1D(disp);
						break;
					case 2:
						theNode->SetDisplacements2D(disp);
						break;
					case 3:
						theNode->SetDisplacements3D(disp);
						break;
					default:
						throw MechanicsException("[NuTo::StructureBase::ElementCoefficientMatrix_0_Check] Only nodes with 1,2 or 3 displacement components considered.");
					}
					//update tmpstatic data of nonlocal elements
					if (mHaveTmpStaticData)
					{
						const_cast<NuTo::ElementBase*>(nonlocalElements[countElement])->UpdateStaticData(NuTo::Element::TMPSTATICDATA);
					}
				}
				else
				{
					//reset the displacements and then calculate the resforce and the reset the tmp static data
					disp[countDisp]-=rDelta;
					switch (theNode->GetNumDisplacements())
					{
					case 1:
						theNode->SetDisplacements1D(disp);
						break;
					case 2:
						theNode->SetDisplacements2D(disp);
						break;
					case 3:
						theNode->SetDisplacements3D(disp);
						break;
					default:
						throw MechanicsException("[NuTo::StructureBase::ElementCoefficientMatrix_0_Check] Only nodes with 1,2 or 3 displacement components considered.");
					}

					rElementPtr->CalculateGradientInternalPotential(resforce2, globalDofsRow);

					//update tmpstatic data of nonlocal elements
					if (mHaveTmpStaticData)
					{
						const_cast<NuTo::ElementBase*>(nonlocalElements[countElement])->UpdateStaticData(NuTo::Element::TMPSTATICDATA);
					}

				}

				//assemble the matrix
				if (curCol>=stiffnessCDF.GetNumColumns())
				{
					std::cout << "num nonlocal elements " << nonlocalElements.size() << std::endl;
					std::cout << "cur col " << curCol << " size of matrix " << stiffnessCDF.GetNumColumns() << std::endl;
					throw MechanicsException("[NuTo::StructureBase::ElementCoefficientMatrix_0_Resforce] Allocated matrix for calculation of stiffness with finite differences has illegal size.");
				}
				stiffnessCDF.SetColumn(curCol,((resforce2-resforce1)*(1./rDelta)));
				curCol++;
			}
		}
	}
}

//! @brief calculates the coefficient matrix for the 0-th derivative in the differential equation
//! for a mechanical problem, this corresponds to the stiffness matrix
void NuTo::StructureBase::ElementCoefficientMatrix_0(int rElementId,
		                 NuTo::FullMatrix<double>& rResult,
		                 NuTo::FullMatrix<int>& rGlobalDofsRow,
		                 NuTo::FullMatrix<int>& rGlobalDofsColumn)const
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    // build global tmp static data
    if (this->mHaveTmpStaticData && this->mUpdateTmpStaticDataRequired)
    {
        throw MechanicsException("[NuTo::StructureBase::ElementCoefficientMatrix_0] First update of tmp static data required.");
    }

    const ElementBase* elementPtr = ElementGetElementPtr(rElementId);
    std::vector<int> globalDofsRow,
    		         globalDofsColumn;

    try
    {
         bool symmetryFlag;
    	 elementPtr->CalculateCoefficientMatrix_0(rResult, globalDofsRow, globalDofsColumn, symmetryFlag);
    }
    catch(NuTo::MechanicsException e)
    {
        std::stringstream ss;
        ss << rElementId;
    	e.AddMessage("[NuTo::StructureBase::ElementCoefficientMatrix_0] Error building element matrix for element "
        	+ ss.str() + ".");
        throw e;
    }
    catch(...)
    {
        std::stringstream ss;
        ss << rElementId;
    	throw NuTo::MechanicsException
    	   ("[NuTo::StructureBase::ElementCoefficientMatrix_0] Error building element matrix for element " + ss.str() + ".");
    }

    //cast to FullMatrixInt
    rGlobalDofsRow.Resize(globalDofsRow.size(),1);
    memcpy(rGlobalDofsRow.mEigenMatrix.data(),&globalDofsRow[0],globalDofsRow.size()*sizeof(int));

    rGlobalDofsColumn.Resize(globalDofsColumn.size(),1);
    memcpy(rGlobalDofsColumn.mEigenMatrix.data(),&globalDofsRow[0],globalDofsRow.size()*sizeof(int));
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementCoefficientMatrix_0] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief calculates the gradient of the internal potential
//! for a mechanical problem, this corresponds to the internal force vector
void NuTo::StructureBase::ElementGradientInternalPotential(int rElementId,
		NuTo::FullMatrix<double>& rResult,
		NuTo::FullMatrix<int>& rGlobalDofsRow)const
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    // build global tmp static data
    if (this->mHaveTmpStaticData && this->mUpdateTmpStaticDataRequired)
    {
        throw MechanicsException("[NuTo::StructureBase::ElementGradientInternalPotential] First update of tmp static data required.");
    }

    const ElementBase* elementPtr = ElementGetElementPtr(rElementId);
    std::vector<int> globalDofsRow;

    try
    {
    	 elementPtr->CalculateGradientInternalPotential(rResult, globalDofsRow);
    }
    catch(NuTo::MechanicsException e)
    {
        std::stringstream ss;
        ss << rElementId;
    	e.AddMessage("[NuTo::StructureBase::ElementGradientInternalPotential] Error building element vector for element "
        	+ ss.str() + ".");
        throw e;
    }
    catch(...)
    {
        std::stringstream ss;
        ss << rElementId;
    	throw NuTo::MechanicsException
    	   ("[NuTo::StructureBase::ElementGradientInternalPotential] Error buildung element vector for element " + ss.str() + ".");
    }

    //cast to FullMatrixInt
    rGlobalDofsRow.Resize(globalDofsRow.size(),1);
    memcpy(rGlobalDofsRow.mEigenMatrix.data(),&globalDofsRow[0],globalDofsRow.size()*sizeof(int));

#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementGradientInternalPotential] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}


//! @brief calculates the coefficient matrix for the 1-th derivative in the differential equation
//! for a mechanical problem, this corresponds to the damping matrix
void NuTo::StructureBase::ElementCoefficientMatrix_1(int rElementId,
		     NuTo::FullMatrix<double>& rResult,
             NuTo::FullMatrix<int>& rGlobalDofsRow,
             NuTo::FullMatrix<int>& rGlobalDofsColumn)const
{
	throw MechanicsException("[NuTo::StructureBase::ElementCoefficientMatrix_1] To be implemented.");
}

//! @brief calculates the coefficient matrix for the 2-th derivative in the differential equation
//! for a mechanical problem, this corresponds to the Mass matrix
void NuTo::StructureBase::ElementCoefficientMatrix_2(int rElementId,
		     NuTo::FullMatrix<double>& rResult,
             NuTo::FullMatrix<int>& rGlobalDofsRow,
             NuTo::FullMatrix<int>& rGlobalDofsColumn)const
{
    // build global tmp static data
    if (this->mHaveTmpStaticData && this->mUpdateTmpStaticDataRequired)
    {
        throw MechanicsException("[NuTo::StructureBase::ElementCoefficientMatrix_2] First update of tmp static data required.");
    }

    const ElementBase* elementPtr = ElementGetElementPtr(rElementId);
    std::vector<int> globalDofsRow,
    		         globalDofsColumn;

    try
    {
    	 elementPtr->CalculateCoefficientMatrix_2(rResult, globalDofsRow, globalDofsColumn);
    }
    catch(NuTo::MechanicsException e)
    {
        std::stringstream ss;
        ss << rElementId;
    	e.AddMessage("[NuTo::StructureBase::ElementCoefficientMatrix_2] Error building element matrix for element "
        	+ ss.str() + ".");
        throw e;
    }
    catch(...)
    {
        std::stringstream ss;
        ss << rElementId;
    	throw NuTo::MechanicsException
    	   ("[NuTo::StructureBase::ElementCoefficientMatrix_2] Error building element matrix for element " + ss.str() + ".");
    }

    //cast to FullMatrixInt
    rGlobalDofsRow.Resize(globalDofsRow.size(),1);
    memcpy(rGlobalDofsRow.mEigenMatrix.data(),&globalDofsRow[0],globalDofsRow.size()*sizeof(int));

    rGlobalDofsColumn.Resize(globalDofsColumn.size(),1);
    memcpy(rGlobalDofsColumn.mEigenMatrix.data(),&globalDofsColumn[0],globalDofsColumn.size()*sizeof(int));
}

//! @brief sets the constitutive law of a single element
//! @param rElementIdent identifier for the element
//! @param rConstitutiveLawIdent identifier for the material
void NuTo::StructureBase::ElementSetConstitutiveLaw(int rElementId, int rConstitutiveLawIdent)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    ElementBase* elementPtr = ElementGetElementPtr(rElementId);

    boost::ptr_map<int,ConstitutiveBase>::iterator itConstitutive = mConstitutiveLawMap.find(rConstitutiveLawIdent);
    if (itConstitutive==mConstitutiveLawMap.end())
        throw MechanicsException("[NuTo::StructureBase::ElementSetConstitutiveLaw] Constitutive law with the given identifier does not exist.");

    try
    {
    	ElementSetConstitutiveLaw(elementPtr,itConstitutive->second);
    }
    catch(NuTo::MechanicsException e)
    {
        std::stringstream ss;
        ss << rElementId;
        e.AddMessage("[NuTo::StructureBase::ElementSetConstitutiveLaw] Error setting constitutive law  for element "
        	+ ss.str() + ".");
        throw e;
    }
    catch(...)
    {
        std::stringstream ss;
        ss << rElementId;
    	throw NuTo::MechanicsException
    	   ("[NuTo::StructureBase::ElementSetConstitutiveLaw] Error setting constitutive law  for element " + ss.str() + ".");
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementSetConstitutiveLaw] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief sets the constitutive law of a group of elements
//! @param rGroupIdent identifier for the group of elements
//! @param rConstitutiveLawIdent identifier for the material
void NuTo::StructureBase::ElementGroupSetConstitutiveLaw(int rGroupIdent, int rConstitutiveLawIdent)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
	boost::ptr_map<int,GroupBase>::iterator itGroup = mGroupMap.find(rGroupIdent);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::ElementGroupSetConstitutiveLaw] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=NuTo::Groups::Elements)
    	throw MechanicsException("[NuTo::StructureBase::ElementGroupSetConstitutiveLaw] Group is not an element group.");
    Group<ElementBase> *elementGroup = dynamic_cast<Group<ElementBase>*>(itGroup->second);
    assert(elementGroup!=0);

	boost::ptr_map<int,ConstitutiveBase>::iterator itConstitutive = mConstitutiveLawMap.find(rConstitutiveLawIdent);
    if (itConstitutive==mConstitutiveLawMap.end())
        throw MechanicsException("[NuTo::StructureBase::ElementGroupSetConstitutiveLaw] Constitutive law with the given identifier does not exist.");

    for (Group<ElementBase>::iterator itElement=elementGroup->begin(); itElement!=elementGroup->end();itElement++)
    {
        try
        {
        	ElementSetConstitutiveLaw(*itElement,itConstitutive->second);
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(*itElement);
            e.AddMessage("[NuTo::StructureBase::ElementGroupSetConstitutiveLaw] Error setting constitutive law  for element "
            	+ ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(*itElement);
        	throw NuTo::MechanicsException
        	   ("[NuTo::StructureBase::ElementGroupSetConstitutiveLaw] Error setting constitutive law for element " + ss.str() + ".");
        }
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementGroupSetConstitutiveLaw] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief sets the constitutive law of a all elements
//! @param rConstitutiveLawIdent identifier for the material
void NuTo::StructureBase::ElementTotalSetConstitutiveLaw(int rConstitutiveLawIdent)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    boost::ptr_map<int,ConstitutiveBase>::iterator itConstitutive = mConstitutiveLawMap.find(rConstitutiveLawIdent);
    if (itConstitutive==mConstitutiveLawMap.end())
        throw MechanicsException("[NuTo::StructureBase::ElementTotalSetConstitutiveLaw] Constitutive law with the given identifier does not exist.");

    std::vector<ElementBase*> elementVector;
    GetElementsTotal(elementVector);
    for (unsigned int countElement=0;  countElement<elementVector.size();countElement++)
    {
        try
        {
        	ElementSetConstitutiveLaw(elementVector[countElement],itConstitutive->second);
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[countElement]);
            e.AddMessage("[NuTo::StructureBase::ElementTotalSetConstitutiveLaw] Error setting constitutive law  for element "
            		+ ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[countElement]);
        	throw NuTo::MechanicsException
        	   ("[NuTo::StructureBase::ElementTotalSetConstitutiveLaw] Error setting constitutive law for element "
        			   + ss.str() + ".");
        }
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementTotalSetConstitutiveLaw] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief sets the constitutive law of a single element
//! @param rElement element pointer
//! @param rConstitutive material pointer
void NuTo::StructureBase::ElementSetConstitutiveLaw(ElementBase* rElement, ConstitutiveBase* rConstitutive)
{
    //std::cout<< "[NuTo::StructureBase::ElementSetConstitutiveLaw]" << std::endl;
	rElement->SetConstitutiveLaw(rConstitutive);
}


//! @brief sets the section of a single element
//! @param rElementIdent identifier for the element
//! @param rConstitutiveLawIdent identifier for the section
void NuTo::StructureBase::ElementSetSection(int rElementId, int rSectionId)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    ElementBase* elementPtr = ElementGetElementPtr(rElementId);

    boost::ptr_map<int,SectionBase>::iterator itSection = mSectionMap.find(rSectionId);
    if (itSection==mSectionMap.end())
        throw MechanicsException("[NuTo::StructureBase::ElementSetSection] Section with the given identifier does not exist.");

    try
    {
    	ElementSetSection(elementPtr,itSection->second);
    }
    catch(NuTo::MechanicsException e)
    {
        std::stringstream ss;
        ss << rElementId;
        e.AddMessage("[NuTo::StructureBase::ElementSetSection] Error setting section for element "
        	+ ss.str() + ".");
        throw e;
    }
    catch(...)
    {
        std::stringstream ss;
        ss << rElementId;
    	throw NuTo::MechanicsException
    	   ("[NuTo::StructureBase::ElementSetSection] Error setting section for element " + ss.str() + ".");
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementSetSection] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief sets the section of a group of elements
//! @param rGroupIdent identifier for the group of elements
//! @param rConstitutiveLawIdent identifier for the material
void NuTo::StructureBase::ElementGroupSetSection(int rGroupIdent, int rSectionId)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
	boost::ptr_map<int,GroupBase>::iterator itGroup = mGroupMap.find(rGroupIdent);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::ElementGroupSetConstitutiveLaw] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=NuTo::Groups::Elements)
    	throw MechanicsException("[NuTo::StructureBase::ElementGroupSetConstitutiveLaw] Group is not an element group.");
    Group<ElementBase> *elementGroup = dynamic_cast<Group<ElementBase>*>(itGroup->second);
    assert(elementGroup!=0);

	boost::ptr_map<int,SectionBase>::iterator itSection = mSectionMap.find(rSectionId);
    if (itSection==mSectionMap.end())
        throw MechanicsException("[NuTo::StructureBase::ElementGroupSetConstitutiveLaw] Section with the given identifier does not exist.");

    for (Group<ElementBase>::iterator itElement=elementGroup->begin(); itElement!=elementGroup->end();itElement++)
    {
        try
        {
        	ElementSetSection(*itElement,itSection->second);
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(*itElement);
            e.AddMessage("[NuTo::StructureBase::ElementGroupSetConstitutiveLaw] Error setting section for element "
            	+ ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(*itElement);
        	throw NuTo::MechanicsException
        	   ("[NuTo::StructureBase::ElementGroupSetConstitutiveLaw] Error setting section for element " + ss.str() + ".");
        }
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementGroupSetConstitutiveLaw] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief sets the section for all elements
//! @param rConstitutiveLawIdent identifier for the material
void NuTo::StructureBase::ElementTotalSetSection(int rSectionId)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    boost::ptr_map<int,SectionBase>::iterator itSection = mSectionMap.find(rSectionId);
    if (itSection==mSectionMap.end())
        throw MechanicsException("[NuTo::StructureBase::ElementTotalSetConstitutiveLaw] Section with the given identifier does not exist.");

    std::vector<ElementBase*> elementVector;
    GetElementsTotal(elementVector);
    for (unsigned int countElement=0;  countElement<elementVector.size();countElement++)
    {
        try
        {
        	ElementSetSection(elementVector[countElement],itSection->second);
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[countElement]);
            e.AddMessage("[NuTo::StructureBase::ElementTotalSetConstitutiveLaw] Error setting section  for element "
            		+ ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[countElement]);
        	throw NuTo::MechanicsException
        	   ("[NuTo::StructureBase::ElementTotalSetConstitutiveLaw] Error setting section for element "
        			   + ss.str() + ".");
        }
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementTotalSetConstitutiveLaw] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief modifies the material of a single element
//! @param rElement element pointer
//! @param rConstitutive material pointer
void NuTo::StructureBase::ElementSetSection(ElementBase* rElement, SectionBase* rSection)
{
    rElement->SetSection(rSection);
}

//! @brief returns the enum of string identifier for an integration type
//! @param rIpDataTypeStr string
//! @return enum
NuTo::IpData::eIpDataType NuTo::StructureBase::ElementGetEnumIntegrationType(const std::string& rIpDataTypeStr)
{
    // get ip data type
    std::string upperCaseIpDataTypeStr;
    std::transform(rIpDataTypeStr.begin(), rIpDataTypeStr.end(), std::back_inserter(upperCaseIpDataTypeStr), (int(*)(int)) toupper);

    NuTo::IpData::eIpDataType ipDataType;
    if (upperCaseIpDataTypeStr=="NOIPDATA")
    {
    	ipDataType = NuTo::IpData::NOIPDATA;
    }
    else if (upperCaseIpDataTypeStr=="STATICDATA")
	{
    	ipDataType = NuTo::IpData::STATICDATA;
	}
    else if (upperCaseIpDataTypeStr=="STATICDATANONLOCAL")
    {
    	ipDataType = NuTo::IpData::STATICDATANONLOCAL;
    }
    else
    {
    	throw MechanicsException("[NuTo::Structure::ElementGetEnumIntegrationType] Ip data type "+upperCaseIpDataTypeStr +" does not exist.");
    }
    return ipDataType;
}


//! @brief modifies the section of a single element
//! @param rElementIdent identifier for the element
//! @param rSectionIdent identifier for the section
void NuTo::StructureBase::ElementSetIntegrationType(int rElementId,
		const std::string& rIntegrationTypeIdent, std::string rIpDataTypeStr)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    ElementBase* elementPtr = ElementGetElementPtr(rElementId);

    try
    {
    	ElementSetIntegrationType(elementPtr,GetPtrIntegrationType(rIntegrationTypeIdent), ElementGetEnumIntegrationType(rIpDataTypeStr));
    }
    catch(NuTo::MechanicsException e)
    {
        std::stringstream ss;
        ss << rElementId;
        e.AddMessage("[NuTo::StructureBase::ElementSetIntegrationType] Error setting integration type for element "
        	+ ss.str() + ".");
        throw e;
    }
    catch(...)
    {
        std::stringstream ss;
        ss << rElementId;
    	throw NuTo::MechanicsException
    	   ("[NuTo::StructureBase::ElementSetIntegrationType] Error setting integration type for element " + ss.str() + ".");
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementSetIntegrationType] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif

}

//! @brief modifies the integration type of a group of elements
//! @param rGroupIdent identifier for the group of elements
//! @param rIntegrationTypeIdent identifier for the integration type
void NuTo::StructureBase::ElementGroupSetIntegrationType(int rGroupIdent,
		const std::string& rIntegrationTypeIdent, std::string rIpDataTypeStr)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
	boost::ptr_map<int,GroupBase>::iterator itGroup = mGroupMap.find(rGroupIdent);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::ElementGroupSetIntegrationType] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=NuTo::Groups::Elements)
    	throw MechanicsException("[NuTo::StructureBase::ElementGroupSetIntegrationType] Group is not an element group.");
    Group<ElementBase> *elementGroup = dynamic_cast<Group<ElementBase>*>(itGroup->second);
    assert(elementGroup!=0);
    NuTo::IpData::eIpDataType ipDataType = ElementGetEnumIntegrationType(rIpDataTypeStr);
    for (Group<ElementBase>::iterator itElement=elementGroup->begin(); itElement!=elementGroup->end();itElement++)
    {
        try
        {
        	ElementSetIntegrationType(*itElement,GetPtrIntegrationType(rIntegrationTypeIdent),ipDataType);
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(*itElement);
            e.AddMessage("[NuTo::StructureBase::ElementGroupSetIntegrationType] Error setting integration type for element "
            	+ ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(*itElement);
       	    throw NuTo::MechanicsException
        	   ("[NuTo::StructureBase::ElementGroupSetIntegrationType] Error setting integration type for element " + ss.str() + ".");
        }
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementGroupSetIntegrationType] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif

}

//! @brief modifies the section of a all elements
//! @param rSectionIdent identifier for the section
void NuTo::StructureBase::ElementTotalSetIntegrationType(const std::string& rIntegrationTypeIdent, std::string rIpDataTypeStr)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    NuTo::IpData::eIpDataType ipDataType = ElementGetEnumIntegrationType(rIpDataTypeStr);
    std::vector<ElementBase*> elementVector;
    GetElementsTotal(elementVector);
    for (unsigned int countElement=0;  countElement<elementVector.size();countElement++)
    {
        try
        {
        	ElementSetIntegrationType(elementVector[countElement],GetPtrIntegrationType(rIntegrationTypeIdent),ipDataType);
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[countElement]);
            e.AddMessage("[NuTo::StructureBase::ElementTotalSetIntegrationType] Error setting integration type for element "
            		+ ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[countElement]);
        	throw NuTo::MechanicsException
        	   ("[NuTo::StructureBase::ElementTotalSetIntegrationType] Error setting integration type for element "
        			   + ss.str() + ".");
        }
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementTotalSetIntegrationType] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief modifies the integration type of a single element
//! @param rElement element pointer
//! @param rIntegrationType integration type
void NuTo::StructureBase::ElementSetIntegrationType(ElementBase* rElement, const IntegrationTypeBase* rIntegrationType, NuTo::IpData::eIpDataType rIpDataType)
{
	rElement->SetIntegrationType(rIntegrationType, rIpDataType);
}

//! @brief calculates the engineering strain
//! @param rElemIdent  identifier for the element
//! @param rEngineerungStrain engineering strain (return value, always 6xnumIp matrix)
void NuTo::StructureBase::ElementGetEngineeringStrain(int rElementId, FullMatrix<double>& rEngineeringStrain)const
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    // build global tmp static data
    if (this->mHaveTmpStaticData && this->mUpdateTmpStaticDataRequired)
    {
        throw MechanicsException("[NuTo::StructureBase::ElementGetEngineeringStrain] First update of tmp static data required.");
    }
    const ElementBase* elementPtr = ElementGetElementPtr(rElementId);

    try
    {
    	elementPtr->GetIpData(NuTo::IpData::ENGINEERING_STRAIN, rEngineeringStrain);
    }
    catch(NuTo::MechanicsException e)
    {
        std::stringstream ss;
        ss << rElementId;
        e.AddMessage("[NuTo::StructureBase::ElementGetEngineeringStrain] Error getting engineering strain for element "
        	+ ss.str() + ".");
        throw e;
    }
    catch(...)
    {
        std::stringstream ss;
        ss << rElementId;
    	throw NuTo::MechanicsException
    	   ("[NuTo::StructureBase::ElementGetEngineeringStrain] Error getting engineering strain for element " + ss.str() + ".");
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementGetEngineeringStrain] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief calculates the engineering plastic strain
//! @param rElemIdent  identifier for the element
//! @param rEngineerungStrain engineering plastic strain (return value, always 6xnumIp matrix)
void NuTo::StructureBase::ElementGetEngineeringPlasticStrain(int rElementId, FullMatrix<double>& rEngineeringPlasticStrain)const
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    // build global tmp static data
    if (this->mHaveTmpStaticData && this->mUpdateTmpStaticDataRequired)
    {
        throw MechanicsException("[NuTo::StructureBase::ElementGetEngineeringPlasticStrain] First update of tmp static data required.");
    }
    const ElementBase* elementPtr = ElementGetElementPtr(rElementId);

    try
    {
    	elementPtr->GetIpData(NuTo::IpData::ENGINEERING_PLASTIC_STRAIN, rEngineeringPlasticStrain);
    }
    catch(NuTo::MechanicsException e)
    {
        std::stringstream ss;
        ss << rElementId;
        e.AddMessage("[NuTo::StructureBase::ElementGetEngineeringPlasticStrain] Error getting engineering strain for element "
        	+ ss.str() + ".");
        throw e;
    }
    catch(...)
    {
        std::stringstream ss;
        ss << rElementId;
    	throw NuTo::MechanicsException
    	   ("[NuTo::StructureBase::ElementGetEngineeringPlasticStrain] Error getting engineering strain for element " + ss.str() + ".");
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementGetEngineeringPlasticStrain] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief calculates the engineering stress
//! @param rElemIdent  identifier for the element
//! @param rEngineeringStress engineering stress (return value, always 6xnumIp matrix)
void NuTo::StructureBase::ElementGetEngineeringStress(int rElementId, FullMatrix<double>& rEngineeringStress)const
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    // build global tmp static data
    if (this->mHaveTmpStaticData && this->mUpdateTmpStaticDataRequired)
    {
        throw MechanicsException("[NuTo::StructureBase::ElementGetEngineeringStress] First update of tmp static data required.");
    }

    const ElementBase* elementPtr = ElementGetElementPtr(rElementId);
    try
    {
    	elementPtr->GetIpData(NuTo::IpData::ENGINEERING_STRESS, rEngineeringStress);
    }
    catch(NuTo::MechanicsException e)
    {
        std::stringstream ss;
        ss << rElementId;
        e.AddMessage("[NuTo::StructureBase::ElementGetEngineeringStress] Error getting engineering strain for element "
        	+ ss.str() + ".");
        throw e;
    }
    catch(...)
    {
        std::stringstream ss;
        ss << rElementId;
    	throw NuTo::MechanicsException
    	   ("[NuTo::StructureBase::ElementGetEngineeringStress] Error getting engineering strain for element " + ss.str() + ".");
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementGetEngineeringStress] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief updates the history data of a all elements
void NuTo::StructureBase::ElementTotalUpdateStaticData()
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    // build global tmp static data
    if (this->mHaveTmpStaticData && this->mUpdateTmpStaticDataRequired)
    {
        try
        {
            this->ElementTotalUpdateTmpStaticData();
        }
        catch (MechanicsException& e)
        {
        	e.AddMessage("[NuTo::StructureBase::ElementGetEngineeringStress] error building tmp static data.");
        	throw e;
        }
    }

    std::vector<ElementBase*> elementVector;
    GetElementsTotal(elementVector);
    for (unsigned int countElement=0;  countElement<elementVector.size();countElement++)
    {
        try
        {
        	elementVector[countElement]->UpdateStaticData(NuTo::Element::STATICDATA);
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[countElement]);
            e.AddMessage("[NuTo::StructureBase::ElementTotalUpdateStaticData] Error updating static data for element "
            		+ ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[countElement]);
        	throw NuTo::MechanicsException
        	   ("[NuTo::StructureBase::ElementTotalUpdateStaticData] Error updating static data for element "
        			   + ss.str() + ".");
        }
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementTotalUpdateStaticData] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief updates the history data of a all elements
void NuTo::StructureBase::ElementTotalUpdateTmpStaticData()
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
	if (mHaveTmpStaticData)
	{
		std::vector<ElementBase*> elementVector;
		GetElementsTotal(elementVector);
		for (unsigned int countElement=0;  countElement<elementVector.size();countElement++)
		{
			try
			{
				elementVector[countElement]->UpdateStaticData(NuTo::Element::TMPSTATICDATA);
			}
			catch(NuTo::MechanicsException e)
			{
				std::stringstream ss;
				ss << ElementGetId(elementVector[countElement]);
				e.AddMessage("[NuTo::StructureBase::ElementTotalUpdateTmpStaticData] Error updating temporary static data for element "
						+ ss.str() + ".");
				throw e;
			}
			catch(...)
			{
				std::stringstream ss;
				ss << ElementGetId(elementVector[countElement]);
				throw NuTo::MechanicsException
				   ("[NuTo::StructureBase::ElementTotalUpdateTmpStaticData] Error updating temporary static data for element "
						   + ss.str() + ".");
			}
		}
	}
	//std::cout << "NuTo::StructureBase::ElementTotalUpdateTmpStaticData " << mUpdateTmpStaticDataRequired << std::endl;
	mUpdateTmpStaticDataRequired = false;
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementTotalUpdateTmpStaticData] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief calculates the average stress
//! @param rVolume  volume of the structure in 3D /area in 2D/ length in 1D
//! this is a parameter of the model, since holes have to be considered (zero stress, but still nonzero area)
//! @param rEngineeringStress  average stress (return value)
void NuTo::StructureBase::ElementTotalGetAverageStress(double rVolume, NuTo::FullMatrix<double>& rEngineeringStress)const
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    NuTo::FullMatrix<double> elementEngineeringStress;
    rEngineeringStress.Resize(6,1);

    std::vector<const ElementBase*> elementVector;
    GetElementsTotal(elementVector);
    for (unsigned int elementCount=0; elementCount<elementVector.size();elementCount++)
    {
        try
        {
            elementVector[elementCount]->GetIntegratedStress(elementEngineeringStress);
            rEngineeringStress+=elementEngineeringStress;
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[elementCount]);
            e.AddMessage("[NuTo::StructureBase::ElementTotalGetAverageStress] Error calculating integrated stress  for element "  + ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[elementCount]);
            throw NuTo::MechanicsException
               ("[NuTo::StructureBase::ElementTotalGetAverageStress] Error calculating integrated stress  for element " + ss.str() + ".");
        }
    }
    rEngineeringStress*=1./rVolume;

#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementTotalGetAverageStress] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief calculates the average stress
//! @param rGroupId  group number
//! @param rVolume  volume of the structure in 3D /area in 2D/ length in 1D
//! this is a parameter of the model, since holes have to be considered (zero stress, but still nonzero area)
//! @param rEngineeringStress  average stress (return value)
void NuTo::StructureBase::ElementGroupGetAverageStress(int rGroupId, double rVolume, NuTo::FullMatrix<double>& rEngineeringStress)const
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif

    boost::ptr_map<int,GroupBase>::const_iterator itGroup = mGroupMap.find(rGroupId);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::ElementGroupGetAverageStress] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=NuTo::Groups::Elements)
        throw MechanicsException("[NuTo::StructureBase::ElementGroupGetAverageStress] Group is not an element group.");
    const Group<ElementBase> *elementGroup = dynamic_cast<const Group<ElementBase>*>(itGroup->second);
    assert(elementGroup!=0);

    NuTo::FullMatrix<double> elementEngineeringStress;
    rEngineeringStress.Resize(6,1);

    for (Group<ElementBase>::iterator itElement=elementGroup->begin(); itElement!=elementGroup->end();itElement++)
    {
        try
        {
            (*itElement)->GetIntegratedStress(elementEngineeringStress);
            rEngineeringStress+=elementEngineeringStress;
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(*itElement);
            e.AddMessage("[NuTo::StructureBase::ElementGroupGetAverageStress] Error calculating integrated stress  for element "  + ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(*itElement);
            throw NuTo::MechanicsException
               ("[NuTo::StructureBase::ElementGroupGetAverageStress] Error calculating integrated stress  for element " + ss.str() + ".");
        }
    }
    rEngineeringStress*=(1./rVolume);


#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementGroupGetAverageStress] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}


//! @brief calculates the average strain
//! @param rVolume  volume of the structure in 3D /area in 2D/ length in 1D
//! this is a parameter of the model, since holes have to be considered (zero strain, but still nonzero area)
//! @param rEngineeringStraiu  average strain (return value)
void NuTo::StructureBase::ElementTotalGetAverageStrain(double rVolume, NuTo::FullMatrix<double>& rEngineeringStrain)const
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    NuTo::FullMatrix<double> elementEngineeringStrain;
    rEngineeringStrain.Resize(6,1);

    std::vector<const ElementBase*> elementVector;
    GetElementsTotal(elementVector);
    for (unsigned int elementCount=0; elementCount<elementVector.size();elementCount++)
    {
        try
        {
            elementVector[elementCount]->GetIntegratedStrain(elementEngineeringStrain);
            rEngineeringStrain+=elementEngineeringStrain;
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[elementCount]);
            e.AddMessage("[NuTo::StructureBase::ElementTotalGetAverageStrain] Error calculating integrated strain  for element "  + ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[elementCount]);
            throw NuTo::MechanicsException
               ("[NuTo::StructureBase::ElementTotalGetAverageStrain] Error calculating integrated strain  for element " + ss.str() + ".");
        }
    }
    rEngineeringStrain*=1./rVolume;

#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementTotalGetAverageStrain] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}

//! @brief calculates the average strain
//! @param rGroupId  group number
//! @param rVolume  volume of the structure in 3D /area in 2D/ length in 1D
//! this is a parameter of the model, since holes have to be considered (zero strain, but still nonzero area)
//! @param rEngineeringStrain  average strain (return value)
void NuTo::StructureBase::ElementGroupGetAverageStrain(int rGroupId, double rVolume, NuTo::FullMatrix<double>& rEngineeringStrain)const
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif

    boost::ptr_map<int,GroupBase>::const_iterator itGroup = mGroupMap.find(rGroupId);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::ElementGroupGetAverageStrain] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=NuTo::Groups::Elements)
        throw MechanicsException("[NuTo::StructureBase::ElementGroupGetAverageStrain] Group is not an element group.");
    const Group<ElementBase> *elementGroup = dynamic_cast<const Group<ElementBase>*>(itGroup->second);
    assert(elementGroup!=0);

    NuTo::FullMatrix<double> elementEngineeringStrain;
    rEngineeringStrain.Resize(6,1);

    for (Group<ElementBase>::iterator itElement=elementGroup->begin(); itElement!=elementGroup->end();itElement++)
    {
        try
        {
            (*itElement)->GetIntegratedStrain(elementEngineeringStrain);
            rEngineeringStrain+=elementEngineeringStrain;
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(*itElement);
            e.AddMessage("[NuTo::StructureBase::ElementGroupGetAverageStrain] Error calculating integrated strain  for element "  + ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(*itElement);
            throw NuTo::MechanicsException
               ("[NuTo::StructureBase::ElementGroupGetAverageStrain] Error calculating integrated strain  for element " + ss.str() + ".");
        }
    }
    rEngineeringStrain*=(1./rVolume);


#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementGroupGetAverageStrain] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
}
//! @brief calculates the total energy of the system
//! @return total energy
double NuTo::StructureBase::ElementTotalGetTotalEnergy()const
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    double totalEnergy(0);
    NuTo::FullMatrix<double> ipEnergy;

    std::vector<const ElementBase*> elementVector;
    GetElementsTotal(elementVector);
    for (unsigned int elementCount=0; elementCount<elementVector.size();elementCount++)
    {
        try
        {
            elementVector[elementCount]->GetIpData(NuTo::IpData::TOTAL_ENERGY,ipEnergy);
            for (int theIP=0; theIP<ipEnergy.GetNumColumns(); theIP++)
            {
                totalEnergy+=ipEnergy(0,theIP)*ipEnergy(1,theIP);
            }
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[elementCount]);
            e.AddMessage("[NuTo::StructureBase::ElementTotalGetAverageStrain] Error calculating integrated strain  for element "  + ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[elementCount]);
            throw NuTo::MechanicsException
               ("[NuTo::StructureBase::ElementTotalGetAverageStrain] Error calculating integrated strain  for element " + ss.str() + ".");
        }
    }

#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementTotalGetTotalEnergy] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
    return totalEnergy;
}

//! @brief calculates the elastic energy of the system
//! @return elastic energy
double NuTo::StructureBase::ElementTotalGetElasticEnergy()const
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    double elasticEnergy(0);
    NuTo::FullMatrix<double> ipEnergy;

    std::vector<const ElementBase*> elementVector;
    GetElementsTotal(elementVector);
    for (unsigned int elementCount=0; elementCount<elementVector.size();elementCount++)
    {
        try
        {
            elementVector[elementCount]->GetIpData(NuTo::IpData::ELASTIC_ENERGY,ipEnergy);
            for (int theIP=0; theIP<ipEnergy.GetNumColumns(); theIP++)
            {
                elasticEnergy+=ipEnergy(0,theIP)*ipEnergy(1,theIP);
            }
        }
        catch(NuTo::MechanicsException e)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[elementCount]);
            e.AddMessage("[NuTo::StructureBase::ElementTotalGetAverageStrain] Error calculating integrated strain  for element "  + ss.str() + ".");
            throw e;
        }
        catch(...)
        {
            std::stringstream ss;
            ss << ElementGetId(elementVector[elementCount]);
            throw NuTo::MechanicsException
               ("[NuTo::StructureBase::ElementTotalGetAverageStrain] Error calculating integrated strain  for element " + ss.str() + ".");
        }
    }

#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        std::cout<<"[NuTo::StructureBase::ElementTotalGetElasticEnergy] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << std::endl;
#endif
    return elasticEnergy;
}
