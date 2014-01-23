// $Id$

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif // ENABLE_SERIALIZATION

#include "nuto/base/Logger.h"
#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/constitutive/ConstitutiveBase.h"
#include "nuto/mechanics/constitutive/ConstitutiveInputBase.h"
#include "nuto/mechanics/constitutive/ConstitutiveOutputBase.h"
#include "nuto/mechanics/constitutive/ConstitutiveStaticDataBase.h"
#include "nuto/mechanics/constitutive/ConstitutiveTangentLocal.h"
#include "nuto/mechanics/constitutive/mechanics/ConstitutiveStaticDataDamageViscoPlasticity3D.h"
#include "nuto/mechanics/constitutive/mechanics/Damage.h"
#include "nuto/mechanics/constitutive/mechanics/DeformationGradient1D.h"
#include "nuto/mechanics/constitutive/mechanics/DeformationGradient2D.h"
#include "nuto/mechanics/constitutive/mechanics/DeformationGradient3D.h"
#include "nuto/mechanics/constitutive/mechanics/DamageViscoPlasticityEngineeringStress.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStress1D.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStress2D.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStress3D.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStrain1D.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStrain2D.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStrain3D.h"
#include "nuto/mechanics/elements/ElementBase.h"
#include "nuto/mechanics/sections/SectionBase.h"
#include "nuto/mechanics/sections/SectionEnum.h"
#include "nuto/math/FullMatrix.h"
#include "nuto/math/FullVector.h"

#define sqrt3 1.732050808

NuTo::DamageViscoPlasticityEngineeringStress::DamageViscoPlasticityEngineeringStress() : ConstitutiveBase()
{
	mE = 0.;
	mNu = 0.;
	mRho = 0.;
	mTensileStrength = 0.;
	mCompressiveStrength = 0.;
	mBiaxialCompressiveStrength = 0.;
	SetParametersValid();
}

#ifdef ENABLE_SERIALIZATION
//! @brief serializes the class
//! @param ar         archive
//! @param version    version
template<class Archive>
void NuTo::DamageViscoPlasticityEngineeringStress::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
   std::cout << "start serialize DamageViscoPlasticityEngineeringStress" << std::endl;
#endif
   ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstitutiveBase)
      & BOOST_SERIALIZATION_NVP(mE)
      & BOOST_SERIALIZATION_NVP(mNu)
      & BOOST_SERIALIZATION_NVP(mRho)
      & BOOST_SERIALIZATION_NVP(mThermalExpansionCoefficient)
      & BOOST_SERIALIZATION_NVP(mTensileStrength)
      & BOOST_SERIALIZATION_NVP(mCompressiveStrength)
      & BOOST_SERIALIZATION_NVP(mBiaxialCompressiveStrength);
#ifdef DEBUG_SERIALIZATION
   std::cout << "finish serialize DamageViscoPlasticityEngineeringStress" << std::endl;
#endif
}
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::DamageViscoPlasticityEngineeringStress)
#endif // ENABLE_SERIALIZATION

//! @brief ... evaluate the constitutive relation in 1D
//! @param rElement ... element
//! @param rIp ... integration point
//! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
//! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
NuTo::Error::eError NuTo::DamageViscoPlasticityEngineeringStress::Evaluate1D(ElementBase* rElement, int rIp,
		const std::map<NuTo::Constitutive::Input::eInput, const ConstitutiveInputBase*>& rConstitutiveInput,
		std::map<NuTo::Constitutive::Output::eOutput, ConstitutiveOutputBase*>& rConstitutiveOutput)
{
    throw NuTo::MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::Evaluate1D] not implemented for 1D.");
}
// Case of Elasticity 1D;
//{
//	// get section information determining which input on the constitutive level should be used
//	const SectionBase* section(rElement->GetSection());
//
//	// check if parameters are valid
//    if (this->mParametersValid == false)
//    {
//   		//throw an exception giving information related to the wrong parameter
//    	CheckParameters();
//    }
//
//	EngineeringStrain1D engineeringStrain;
//	// calculate engineering strain
//	if(rConstitutiveInput.find(NuTo::Constitutive::Input::DEFORMATION_GRADIENT_1D)==rConstitutiveInput.end())
//		throw MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::Evaluate] deformation gradient 1d needed to evaluate engineering strain2d.");
//	const DeformationGradient1D& deformationGradient(rConstitutiveInput.find(NuTo::Constitutive::Input::DEFORMATION_GRADIENT_1D)->second->GetDeformationGradient1D());
//	deformationGradient.GetEngineeringStrain(engineeringStrain);
//
//    for (std::map<NuTo::Constitutive::Output::eOutput, ConstitutiveOutputBase*>::iterator itOutput = rConstitutiveOutput.begin();
//    		itOutput != rConstitutiveOutput.end(); itOutput++)
//    {
//    	switch(itOutput->first)
//    	{
//    	case NuTo::Constitutive::Output::ENGINEERING_STRESS_1D:
//    	{
//    		EngineeringStrain1D elasticEngineeringStrain(engineeringStrain);
//    		// if temperature is an input, subtract thermal strains to get elastic strains
//    		if (section->GetInputConstitutiveIsTemperature())
//    		{
//    			std::map<NuTo::Constitutive::Input::eInput, const ConstitutiveInputBase*>::const_iterator itInput(rConstitutiveInput.find(NuTo::Constitutive::Input::TEMPERATURE));
//    			if (itInput==rConstitutiveInput.end())
//    				throw MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::Evaluate1D] temperature needed to evaluate thermal engineering strain1d.");
//    			double temperature(itInput->second->GetTemperature());
//    			double deltaStrain(mThermalExpansionCoefficient * temperature);
//    			EngineeringStrain1D elasticEngineeringStrain;
//    			elasticEngineeringStrain[0] -= deltaStrain;
//    		}
//			EngineeringStress1D& engineeringStress(itOutput->second->GetEngineeringStress1D());
//			// calculate Engineering stress
//			engineeringStress = mE*elasticEngineeringStrain;
//
//		    break;
//    	}
//    	case NuTo::Constitutive::Output::ENGINEERING_STRESS_3D:
//    	{
//    		//this is for the visualize routines
//    		EngineeringStrain1D elasticEngineeringStrain(engineeringStrain);
//    		// if temperature is an input, subtract thermal strains to get elastic strains
//    		if (section->GetInputConstitutiveIsTemperature())
//    		{
//    			std::map<NuTo::Constitutive::Input::eInput, const ConstitutiveInputBase*>::const_iterator itInput(rConstitutiveInput.find(NuTo::Constitutive::Input::TEMPERATURE));
//    			if (itInput==rConstitutiveInput.end())
//    				throw MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::Evaluate2D] temperature needed to evaluate thermal engineering strain2d.");
//    			double temperature(itInput->second->GetTemperature());
//    			double deltaStrain(mThermalExpansionCoefficient * temperature);
//    			EngineeringStrain1D elasticEngineeringStrain;
//    			elasticEngineeringStrain[0] -= deltaStrain;
//    		}
//			EngineeringStress3D& engineeringStress(itOutput->second->GetEngineeringStress3D());
//
//			// calculate Engineering stress
//			engineeringStress[0] = mE * elasticEngineeringStrain[0];
//			engineeringStress[1] = 0.;
//			engineeringStress[2] = 0.;
//			engineeringStress[3] = 0.;
//			engineeringStress[4] = 0.;
//			engineeringStress[5] = 0.;
//
//		    break;
//    	}
//    	case NuTo::Constitutive::Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN_1D:
//    	{
//			ConstitutiveTangentLocal<1,1>& tangent(itOutput->second->AsConstitutiveTangentLocal_1x1());
// 		    tangent(0,0)=mE;
//		    tangent.SetSymmetry(true);
//    		break;
//    	}
//    	case NuTo::Constitutive::Output::ENGINEERING_STRAIN_1D:
//    	{
//    		EngineeringStrain1D& engineeringStrain1D(itOutput->second->GetEngineeringStrain1D());
//			engineeringStrain1D[0] = engineeringStrain[0];
//    	}
//    	break;
//    	case NuTo::Constitutive::Output::ENGINEERING_STRAIN_3D:
//    	{
//    		EngineeringStrain3D& engineeringStrain3D(itOutput->second->GetEngineeringStrain3D());
//			engineeringStrain3D[0] = engineeringStrain[0];
//			engineeringStrain3D[1] = -mNu*engineeringStrain[0];
//			engineeringStrain3D[2] = engineeringStrain3D[1];
//			engineeringStrain3D[3] = 0.;
//			engineeringStrain3D[4] = 0.;
//			engineeringStrain3D[5] = 0.;
//    	}
//    	break;
//    	case NuTo::Constitutive::Output::ENGINEERING_PLASTIC_STRAIN_3D:
//    	{
//    		EngineeringStrain3D& engineeringPlasticStrain(itOutput->second->GetEngineeringStrain3D());
//    		engineeringPlasticStrain[0] = 0.;
//    		engineeringPlasticStrain[1] = 0.;
//    		engineeringPlasticStrain[2] = 0.;
//    		engineeringPlasticStrain[3] = 0.;
//    		engineeringPlasticStrain[4] = 0.;
//    		engineeringPlasticStrain[5] = 0.;
//    		break;
//    	}
//    	case NuTo::Constitutive::Output::DAMAGE:
//    	{
//    		itOutput->second->GetDamage().SetDamage(0.);
//    		break;
//    	}
//    	case NuTo::Constitutive::Output::UPDATE_TMP_STATIC_DATA:
//    	case NuTo::Constitutive::Output::UPDATE_STATIC_DATA:
//    	{
//    	    //nothing to be done for update routine
//    		break;
//    	}
//    	default:
//    		throw MechanicsException(std::string("[NuTo::DamageViscoPlasticityEngineeringStress::Evaluate1D] output object)") +
//    				NuTo::Constitutive::OutputToString(itOutput->first) +
//    				std::string(" could not be calculated, check the allocated material law and the section behavior."));
//    	}
//    }
//
//    //update history variables but for linear elastic, there is nothing to do
//
//	return Error::SUCCESSFUL;
//}

//! @brief ... evaluate the constitutive relation in 2D
//! @param rElement ... element
//! @param rIp ... integration point
//! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
//! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
NuTo::Error::eError NuTo::DamageViscoPlasticityEngineeringStress::Evaluate2D(ElementBase* rElement, int rIp,
		const std::map<NuTo::Constitutive::Input::eInput, const ConstitutiveInputBase*>& rConstitutiveInput,
		std::map<NuTo::Constitutive::Output::eOutput, ConstitutiveOutputBase*>& rConstitutiveOutput)
{
    throw NuTo::MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::Evaluate2D] not implemented for 2D.");
}
// Case of Elasticity 2D;
//{
//	// get section information determining which input on the constitutive level should be used
//	const SectionBase* section(rElement->GetSection());
//
//	// check if parameters are valid
//    if (this->mParametersValid == false)
//    {
//   		//throw an exception giving information related to the wrong parameter
//    	CheckParameters();
//    }
//
//	EngineeringStrain2D engineeringStrain;
//	// calculate engineering strain
//	if(rConstitutiveInput.find(NuTo::Constitutive::Input::DEFORMATION_GRADIENT_2D)==rConstitutiveInput.end())
//		throw MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::Evaluate] deformation gradient 2d needed to evaluate engineering strain2d.");
//	const DeformationGradient2D& deformationGradient(rConstitutiveInput.find(NuTo::Constitutive::Input::DEFORMATION_GRADIENT_2D)->second->GetDeformationGradient2D());
//	deformationGradient.GetEngineeringStrain(engineeringStrain);
//
//    for (std::map<NuTo::Constitutive::Output::eOutput, ConstitutiveOutputBase*>::iterator itOutput = rConstitutiveOutput.begin();
//    		itOutput != rConstitutiveOutput.end(); itOutput++)
//    {
//    	switch(itOutput->first)
//    	{
//    	case NuTo::Constitutive::Output::ENGINEERING_STRESS_2D:
//    	{
//    		EngineeringStrain2D elasticEngineeringStrain(engineeringStrain);
//    		// if temperature is an input, subtract thermal strains to get elastic strains
//    		if (section->GetInputConstitutiveIsTemperature())
//    		{
//    			std::map<NuTo::Constitutive::Input::eInput, const ConstitutiveInputBase*>::const_iterator itInput(rConstitutiveInput.find(NuTo::Constitutive::Input::TEMPERATURE));
//    			if (itInput==rConstitutiveInput.end())
//    				throw MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::Evaluate2D] temperature needed to evaluate thermal engineering strain2d.");
//    			double temperature(itInput->second->GetTemperature());
//    			double deltaStrain(mThermalExpansionCoefficient * temperature);
//    			EngineeringStrain2D elasticEngineeringStrain;
//    			elasticEngineeringStrain[0] -= deltaStrain;
//    			elasticEngineeringStrain[1] -= deltaStrain;
//    		}
//			EngineeringStress2D& engineeringStress(itOutput->second->GetEngineeringStress2D());
//		    // calculate Engineering stress
//
//		    switch(rElement->GetSection()->GetType())
//		    {
//		    case Section::PLANE_STRAIN:{
//				// calculate coefficients of the material matrix
//				double C11, C12, C33;
//				this->CalculateCoefficients3D(C11, C12, C33);
//
//				// calculate Engineering stress
//				engineeringStress[0] = C11 * elasticEngineeringStrain[0] + C12 * elasticEngineeringStrain[1];
//				engineeringStress[1] = C11 * elasticEngineeringStrain[1] + C12 * elasticEngineeringStrain[0];
//				engineeringStress[2] = C33 * elasticEngineeringStrain[2] ;
//		    	break;}
//		    case Section::PLANE_STRESS:{
//				// calculate coefficients of the material matrix
//				double C11, C12, C33;
//				this->CalculateCoefficients2DPlainStress(C11, C12, C33);
//
//				// calculate Engineering stress
//				engineeringStress[0] = C11 * elasticEngineeringStrain[0] + C12 * elasticEngineeringStrain[1];
//				engineeringStress[1] = C11 * elasticEngineeringStrain[1] + C12 * elasticEngineeringStrain[0];
//				engineeringStress[2] = C33 * elasticEngineeringStrain[2] ;
//		    	break;}
//		    default:
//		    	throw MechanicsException("[NuTo::LinearElastic::GetEngineeringStressFromEngineeringStrain] Invalid type of 2D section behavoir found!!!");
//		    }
//
//
//		    break;
//    	}
//    	case NuTo::Constitutive::Output::ENGINEERING_STRESS_3D:
//    	{
//    		//this is for the visualize routines
//    		EngineeringStrain2D elasticEngineeringStrain(engineeringStrain);
//    		// if temperature is an input, subtract thermal strains to get elastic strains
//    		if (section->GetInputConstitutiveIsTemperature())
//    		{
//    			std::map<NuTo::Constitutive::Input::eInput, const ConstitutiveInputBase*>::const_iterator itInput(rConstitutiveInput.find(NuTo::Constitutive::Input::TEMPERATURE));
//    			if (itInput==rConstitutiveInput.end())
//    				throw MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::Evaluate2D] temperature needed to evaluate thermal engineering strain2d.");
//    			double temperature(itInput->second->GetTemperature());
//    			double deltaStrain(mThermalExpansionCoefficient * temperature);
//    			EngineeringStrain2D elasticEngineeringStrain;
//    			elasticEngineeringStrain[0] -= deltaStrain;
//    			elasticEngineeringStrain[1] -= deltaStrain;
//    		}
//			EngineeringStress3D& engineeringStress(itOutput->second->GetEngineeringStress3D());
//		    // calculate Engineering stress
//
//		    switch(rElement->GetSection()->GetType())
//		    {
//		    case Section::PLANE_STRAIN:{
//				// calculate coefficients of the material matrix
//				double C11, C12, C33;
//				this->CalculateCoefficients3D(C11, C12, C33);
//
//				// calculate Engineering stress
//				engineeringStress[0] = C11 * elasticEngineeringStrain[0] + C12 * elasticEngineeringStrain[1];
//				engineeringStress[1] = C11 * elasticEngineeringStrain[1] + C12 * elasticEngineeringStrain[0];
//				engineeringStress[2] = C12 * (elasticEngineeringStrain[0]+elasticEngineeringStrain[1]);
//				engineeringStress[3] = C33 * elasticEngineeringStrain[2] ;
//				engineeringStress[4] = 0.;
//				engineeringStress[5] = 0.;
//		    	break;}
//		    case Section::PLANE_STRESS:{
//				// calculate coefficients of the material matrix
//				double C11, C12, C33;
//				this->CalculateCoefficients2DPlainStress(C11, C12, C33);
//
//				// calculate Engineering stress
//				engineeringStress[0] = C11 * elasticEngineeringStrain[0] + C12 * elasticEngineeringStrain[1];
//				engineeringStress[1] = C11 * elasticEngineeringStrain[1] + C12 * elasticEngineeringStrain[0];
//				engineeringStress[2] = 0.;
//				engineeringStress[3] = C33 * elasticEngineeringStrain[2] ;
//				engineeringStress[4] = 0.;
//				engineeringStress[5] = 0.;
//		    	break;}
//		    default:
//		    	throw MechanicsException("[NuTo::LinearElastic::GetEngineeringStressFromEngineeringStrain] Invalid type of 2D section behavoir found!!!");
//		    }
//
//
//		    break;
//    	}
//    	case NuTo::Constitutive::Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN_2D:
//    	{
//			ConstitutiveTangentLocal<3,3>& tangent(itOutput->second->AsConstitutiveTangentLocal_3x3());
//
// 			assert(rElement->GetSection()!=0);
// 		    switch(rElement->GetSection()->GetType())
// 		    {
// 		    case Section::PLANE_STRAIN:{
// 			    // calculate coefficients of the material matrix
// 			    double C11, C12, C33;
// 			    this->CalculateCoefficients3D(C11, C12, C33);
//
// 			    // store tangent at the output object
// 			    tangent(0,0) = C11;
// 			    tangent(1,0) = C12;
// 			    tangent(2,0) = 0;
//
// 			    tangent(0,1) = C12;
// 			    tangent(1,1) = C11;
// 			    tangent(2,1) = 0;
//
// 			    tangent(0,2) = 0.;
// 			    tangent(1,2) = 0.;
// 			    tangent(2,2) = C33;
// 		    	break;}
// 		    case Section::PLANE_STRESS:{
// 			    // calculate coefficients of the material matrix
// 			    double C11, C12, C33;
// 				this->CalculateCoefficients2DPlainStress(C11, C12, C33);
//
// 			    // store tangent at the output object
// 			    tangent(0,0) = C11;
// 			    tangent(1,0) = C12;
// 			    tangent(2,0) = 0;
//
// 			    tangent(0,1) = C12;
// 			    tangent(1,1) = C11;
// 			    tangent(2,1) = 0;
//
// 			    tangent(0,2) = 0.;
// 			    tangent(1,2) = 0.;
// 			    tangent(2,2) = C33;
// 		   	break;}
// 		    default:
// 		    	throw MechanicsException("[NuTo::LinearElastic::GetTangent_EngineeringStress_EngineeringStrain] Invalid type of 2D section behavoir found!!!");
// 		    }
//
//		    tangent.SetSymmetry(true);
//    		break;
//    	}
//    	case NuTo::Constitutive::Output::ENGINEERING_STRAIN_3D:
//    	{
//    		EngineeringStrain3D& engineeringStrain3D(itOutput->second->GetEngineeringStrain3D());
//    	    switch(rElement->GetSection()->GetType())
//    	    {
//    	    case Section::PLANE_STRAIN:
//    	    	engineeringStrain3D[0] = engineeringStrain[0];
//    	    	engineeringStrain3D[1] = engineeringStrain[1];
//    	    	engineeringStrain3D[2] = 0;
//    	    	engineeringStrain3D[3] = engineeringStrain[2];
//    	    	engineeringStrain3D[4] = 0.;
//    	    	engineeringStrain3D[5] = 0.;
//    	    	break;
//    	    case Section::PLANE_STRESS:
//    	    	engineeringStrain3D[0] = engineeringStrain[0];
//    	    	engineeringStrain3D[1] = engineeringStrain[1];
//    	    	engineeringStrain3D[2] = mNu/(mNu-1.)*(engineeringStrain[0]+engineeringStrain[1]);
//    	    	engineeringStrain3D[3] = engineeringStrain[2];
//    	    	engineeringStrain3D[4] = 0.;
//    	    	engineeringStrain3D[5] = 0.;
//    	    	break;
//    	    default:
//    	    	throw MechanicsException("[NuTo::LinearElastic::GetEngineeringStrainFromEngineeringStrain] Invalid type of 2D section behavoir found!!!");
//    	    }
//    	}
//    	break;
//    	case NuTo::Constitutive::Output::ENGINEERING_PLASTIC_STRAIN_3D:
//    	{
//    		EngineeringStrain3D& engineeringPlasticStrain(itOutput->second->GetEngineeringStrain3D());
//    		engineeringPlasticStrain[0] = 0.;
//    		engineeringPlasticStrain[1] = 0.;
//    		engineeringPlasticStrain[2] = 0.;
//    		engineeringPlasticStrain[3] = 0.;
//    		engineeringPlasticStrain[4] = 0.;
//    		engineeringPlasticStrain[5] = 0.;
//    		break;
//    	}
//    	case NuTo::Constitutive::Output::DAMAGE:
//    	{
//    		itOutput->second->GetDamage().SetDamage(0.);
//    		break;
//    	}
//    	case NuTo::Constitutive::Output::UPDATE_TMP_STATIC_DATA:
//    	case NuTo::Constitutive::Output::UPDATE_STATIC_DATA:
//    	{
//    	    //nothing to be done for update routine
//    		break;
//    	}
//    	default:
//    		throw MechanicsException(std::string("[NuTo::DamageViscoPlasticityEngineeringStressEngineeringStress::Evaluate3D] output object)") +
//    				NuTo::Constitutive::OutputToString(itOutput->first) +
//    				std::string(" culd not be calculated, check the allocated material law and the section behavior."));
//    	}
//    }
//
//    //update history variables but for linear elastic, there is nothing to do
//
//	return Error::SUCCESSFUL;
//}

//! @brief ... evaluate the constitutive relation in 3D
//! @param rElement ... element
//! @param rIp ... integration point
//! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
//! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
NuTo::Error::eError NuTo::DamageViscoPlasticityEngineeringStress::Evaluate3D(ElementBase* rElement, int rIp,
		const std::map<NuTo::Constitutive::Input::eInput, const ConstitutiveInputBase*>& rConstitutiveInput,
		std::map<NuTo::Constitutive::Output::eOutput, ConstitutiveOutputBase*>& rConstitutiveOutput)
{
// THIS IS TEST NEWTON
//	NuTo::FullVector<double,Eigen::Dynamic> x(3);
//	bool check;
//	NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> (DamageViscoPlasticityEngineeringStress::*fdjacAn)
//			(NuTo::FullVector<double,Eigen::Dynamic>);
//	fdjacAn = &DamageViscoPlasticityEngineeringStress::DResidualAn;
//	x[0] = -1., x[1] = 2., x[2] = 3.;
//	std::cout << x.transpose() << std::endl;
//	this->Newton(x, check, &DamageViscoPlasticityEngineeringStress::DResidualAn);
//	this->Newton(x, check);
//	std::cout << x.transpose() << std::endl;
//	std::cout << check << std::endl;
// THIS IS TEST NEWTON

	// get section information determining which input on the constitutive level should be used
	const SectionBase* section(rElement->GetSection());

	// check if parameters are valid
    if (this->mParametersValid == false)
    {
   		//throw an exception giving information related to the wrong parameter
    	CheckParameters();
    }

    double C11, C12, C44;
	EngineeringStrain3D engineeringStrain;
	// calculate engineering strain
	if(rConstitutiveInput.find(NuTo::Constitutive::Input::DEFORMATION_GRADIENT_3D)==rConstitutiveInput.end())
		throw MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::Evaluate] deformation gradient 3d needed to evaluate engineering strain3d.");
	const DeformationGradient3D& deformationGradient(rConstitutiveInput.find(NuTo::Constitutive::Input::DEFORMATION_GRADIENT_3D)->second->GetDeformationGradient3D());
	deformationGradient.GetEngineeringStrain(engineeringStrain);

	//check, if an nonlinear iteration has to be performed, in this simple case, just calculate the linear elastic coefficients
    if (rConstitutiveOutput.find(NuTo::Constitutive::Output::ENGINEERING_STRESS_3D)!=rConstitutiveOutput.end()
    		|| rConstitutiveOutput.find(NuTo::Constitutive::Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN_3D)!=rConstitutiveOutput.end())
    {
    	//in a nonlinear material routine, the Return mapping would be performed here
		// calculate coefficients of the material matrix
		this->CalculateCoefficients3D(C11, C12, C44);
    }

    for (std::map<NuTo::Constitutive::Output::eOutput, ConstitutiveOutputBase*>::iterator itOutput = rConstitutiveOutput.begin();
    		itOutput != rConstitutiveOutput.end(); itOutput++)
    {
    	switch(itOutput->first)
    	{
    	case NuTo::Constitutive::Output::ENGINEERING_STRESS_3D:
    	{
    		EngineeringStrain3D elasticEngineeringStrain(engineeringStrain);
    		// if temperature is an input, subtract thermal strains to get elastic strains
    		if (section->GetInputConstitutiveIsTemperature())
    		{
    			std::map<NuTo::Constitutive::Input::eInput, const ConstitutiveInputBase*>::const_iterator itInput(rConstitutiveInput.find(NuTo::Constitutive::Input::TEMPERATURE));
    			if (itInput==rConstitutiveInput.end())
    				throw MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::Evaluate] temperature needed to evaluate thermal engineering strain3d.");
    			double temperature(itInput->second->GetTemperature());
    			double deltaStrain(mThermalExpansionCoefficient * temperature);
    			EngineeringStrain3D elasticEngineeringStrain;
    			elasticEngineeringStrain[0] -= deltaStrain;
    			elasticEngineeringStrain[1] -= deltaStrain;
    			elasticEngineeringStrain[2] -= deltaStrain;
    		}
			EngineeringStress3D& engineeringStress(itOutput->second->GetEngineeringStress3D());
		    // calculate Engineering stress

		    engineeringStress[0] = C11 * elasticEngineeringStrain[0] + C12 * (elasticEngineeringStrain[1]+elasticEngineeringStrain[2]);
		    engineeringStress[1] = C11 * elasticEngineeringStrain[1] + C12 * (elasticEngineeringStrain[0]+elasticEngineeringStrain[2]);
		    engineeringStress[2] = C11 * elasticEngineeringStrain[2] + C12 * (elasticEngineeringStrain[0]+elasticEngineeringStrain[1]);
		    engineeringStress[3] = C44 * elasticEngineeringStrain[3] ;
		    engineeringStress[4] = C44 * elasticEngineeringStrain[4] ;
		    engineeringStress[5] = C44 * elasticEngineeringStrain[5] ;
   		break;
    	}
    	case NuTo::Constitutive::Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN_3D:
    	{
			ConstitutiveTangentLocal<6,6>& tangent(itOutput->second->AsConstitutiveTangentLocal_6x6());

 		    tangent(0,0) = C11;
 		    tangent(1,0) = C12;
 		    tangent(2,0) = C12;
 		    tangent(3,0) = 0;
 		    tangent(4,0) = 0;
 		    tangent(5,0) = 0;

 		    tangent(0,1) = C12;
 		    tangent(1,1) = C11;
 		    tangent(2,1) = C12;
 		    tangent(3,1) = 0;
 		    tangent(4,1) = 0;
 		    tangent(5,1) = 0;

 		    tangent(0,2) = C12;
 		    tangent(1,2) = C12;
 		    tangent(2,2) = C11;
 		    tangent(3,2) = 0;
 		    tangent(4,2) = 0;
 		    tangent(5,2) = 0;

 		    tangent(0,3) = 0;
 		    tangent(1,3) = 0;
 		    tangent(2,3) = 0;
 		    tangent(3,3) = C44;
 		    tangent(4,3) = 0;
 		    tangent(5,3) = 0;

 		    tangent(0,4) = 0;
 		    tangent(1,4) = 0;
 		    tangent(2,4) = 0;
 		    tangent(3,4) = 0;
 		    tangent(4,4) = C44;
 		    tangent(5,4) = 0;

 		    tangent(0,5) = 0;
 		    tangent(1,5) = 0;
 		    tangent(2,5) = 0;
 		    tangent(3,5) = 0;
 		    tangent(4,5) = 0;
 		    tangent(5,5) = C44;

		    tangent.SetSymmetry(true);
    		break;
    	}
    	case NuTo::Constitutive::Output::ENGINEERING_STRAIN_3D:
    		itOutput->second->GetEngineeringStrain3D() = engineeringStrain;
    		break;
    	case NuTo::Constitutive::Output::ENGINEERING_PLASTIC_STRAIN_3D:
    	{
    		EngineeringStrain3D& engineeringPlasticStrain(itOutput->second->GetEngineeringStrain3D());
    		engineeringPlasticStrain[0] = 0.;
    		engineeringPlasticStrain[1] = 0.;
    		engineeringPlasticStrain[2] = 0.;
    		engineeringPlasticStrain[3] = 0.;
    		engineeringPlasticStrain[4] = 0.;
    		engineeringPlasticStrain[5] = 0.;
    		break;
    	}
    	case NuTo::Constitutive::Output::DAMAGE:
    	{
    		itOutput->second->GetDamage().SetDamage(0.);
    		break;
    	}
    	case NuTo::Constitutive::Output::UPDATE_TMP_STATIC_DATA:
    	case NuTo::Constitutive::Output::UPDATE_STATIC_DATA:
    	{
    	    //nothing to be done for update routine
    		break;
    	}
    	default:
    		throw MechanicsException(std::string("[NuTo::DamageViscoPlasticityEngineeringStressEngineeringStress::Evaluate3D] output object)") +
    				NuTo::Constitutive::OutputToString(itOutput->first) +
    				std::string(" culd not be calculated, check the allocated material law and the section behavior."));
    	}
    }

    //update history variables but for linear elastic, there is nothing to do

	return Error::SUCCESSFUL;
}

//! @brief ... allocate the correct static data
//! @return ... see brief explanation
NuTo::ConstitutiveStaticDataBase* NuTo::DamageViscoPlasticityEngineeringStress::AllocateStaticDataEngineeringStress_EngineeringStrain1D(const ElementBase* rElement)const
{
	return 0;
}

//! @brief ... allocate the correct static data
//! @return ... see brief explanation
NuTo::ConstitutiveStaticDataBase* NuTo::DamageViscoPlasticityEngineeringStress::AllocateStaticDataEngineeringStress_EngineeringStrain2D(const ElementBase* rElement)const
{
	return 0;
}

//! @brief ... allocate the correct static data
//! @return ... see brief explanation
NuTo::ConstitutiveStaticDataBase* NuTo::DamageViscoPlasticityEngineeringStress::AllocateStaticDataEngineeringStress_EngineeringStrain3D(const ElementBase* rElement)const
{
	return new ConstitutiveStaticDataDamageViscoPlasticity3D();
}

//! @brief ... performs the return mapping procedure in 3D
//! @param rElement ... structure
//! @param rIp ... integration point
//! @param rDeformationGradient ... deformation gradient point
//! @param rNewStress ... new stress (if a 0-pointer is given, no values are written)
//! @param rNewTangent ... new tangent matrix (if a 0-pointer is given, no values are written)
//! @param rNewStaticData ... new static data (if a 0-pointer is given, no values are written)
#define sqrt_2div3 0.81649658
#define tolerance 1e-8
NuTo::Error::eError NuTo::DamageViscoPlasticityEngineeringStress::ReturnMapping3D(const ElementBase* rElement,int rIp,
		const EngineeringStrain3D& rEngineeringStrain,
		EngineeringStress3D* rNewStress,
		ConstitutiveTangentLocal<6,6>* rNewTangent,
		ConstitutiveStaticDataDamageViscoPlasticity3D* rNewStaticData,
		Logger& rLogger)const
{
	// get material parameters
    double f_ct  = mTensileStrength;
    double f_c1  = mCompressiveStrength;
    double f_c2  = mBiaxialCompressiveStrength;

    assert(f_c2>f_c1);
    assert(f_c1>0);
    assert(f_c2>0);

    double rBeta = sqrt3*(f_c2-f_c1) / (2*f_c2-f_c1);
    double rHP  = f_c2*f_c1 / (sqrt3*(2*f_c2-f_c1));

    // get elastic matrix
	// calculate coefficients of the linear elastic material matrix
	NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> ElasticStiffness(6, 6);
	double C11, C12, C44;
	this->CalculateCoefficients3D(C11, C12, C44);
	ElasticStiffness << C11, C12, C12,  0., 0.,  0.,
						C12, C11, C12,  0., 0.,  0.,
						C12, C12, C11,  0., 0.,  0.,
						 0.,  0.,  0., C44, 0.,  0.,
						 0.,  0.,  0.,  0., C44, 0.,
						 0.,  0.,  0.,  0., 0., C44;

    // get state variables to begin of the time increment
    const ConstitutiveStaticDataMisesPlasticity3D* rOldStaticData = rElement->GetStaticData(rIp)->AsConstitutiveStaticDataMisesPlasticity3D();


	return Error::SUCCESSFUL;
}

// calculate coefficients of the material matrix
void NuTo::DamageViscoPlasticityEngineeringStress::CalculateCoefficients2DPlainStress(double& C11, double& C12, double& C33) const
{
    double factor = this->mE/(1.0 - (this->mNu * this->mNu));
    C11 = factor;
    C12 = factor * this->mNu;
    C33 = factor * 0.5 * (1.0 - this->mNu);
}

// calculate coefficients of the material matrix
void NuTo::DamageViscoPlasticityEngineeringStress::CalculateCoefficients3D(double& C11, double& C12, double& C44) const
{
    double factor = this->mE/((1.0 + this->mNu) * (1.0 - 2.0 * this->mNu));
    C11 = factor * (1.0 - this->mNu);
    C12 = factor * this->mNu;
    C44 = this->mE/(2.*(1.0 + this->mNu));
}

///////////////////////////////////////////////////////////////////////////

// parameters /////////////////////////////////////////////////////////////
//! @brief ... get density
//! @return ... density
double NuTo::DamageViscoPlasticityEngineeringStress::GetDensity() const
{
	return this->mRho;
}

//! @brief ... set density
//! @param rRho ... density
void NuTo::DamageViscoPlasticityEngineeringStress::SetDensity(double rRho)
{
    this->CheckDensity(rRho);
    this->mRho = rRho;
    this->SetParametersValid();
}

//! @brief ... get Young's modulus
//! @return ... Young's modulus
double NuTo::DamageViscoPlasticityEngineeringStress::GetYoungsModulus() const
{
	return mE;
}


//! @brief ... set Young's modulus
//! @param rE ... Young's modulus
void NuTo::DamageViscoPlasticityEngineeringStress::SetYoungsModulus(double rE)
{
    this->CheckYoungsModulus(rE);
    this->mE = rE;
    this->SetParametersValid();
}


//! @brief ... get Poisson's ratio
//! @return ... Poisson's ratio
double NuTo::DamageViscoPlasticityEngineeringStress::GetPoissonsRatio() const
{
    return mNu;
}


//! @brief ... set Poisson's ratio
//! @param rNu ... Poisson's ratio
void NuTo::DamageViscoPlasticityEngineeringStress::SetPoissonsRatio(double rNu)
{
    this->CheckPoissonsRatio(rNu);
    this->mNu = rNu;
    this->SetParametersValid();
}

//! @brief ... get thermal expansion coefficient
//! @return ... thermal expansion coefficient
double NuTo::DamageViscoPlasticityEngineeringStress::GetThermalExpansionCoefficient() const
{
    return mThermalExpansionCoefficient;
}

//! @brief ... set thermal expansion coefficient
//! @param rNu ... thermal expansion coefficient
void NuTo::DamageViscoPlasticityEngineeringStress::SetThermalExpansionCoefficient(double rAlpha)
{
    this->CheckThermalExpansionCoefficient(rAlpha);
    this->mThermalExpansionCoefficient = rAlpha;
    this->SetParametersValid();
}

//! @brief ... get tensile strength
//! @return ... tensile strength
double NuTo::DamageViscoPlasticityEngineeringStress::GetTensileStrength() const
{
    return mTensileStrength;
}

//! @brief ... set tensile strength
//! @param rTensileStrength...  tensile strength
void NuTo::DamageViscoPlasticityEngineeringStress::SetTensileStrength(double rTensileStrength)
{
    this->CheckTensileStrength(rTensileStrength);
    this->mTensileStrength = rTensileStrength;
    this->SetParametersValid();
}

//! @brief ... get compressive strength
//! @return ... compressive strength
double NuTo::DamageViscoPlasticityEngineeringStress::GetCompressiveStrength() const
{
    return mCompressiveStrength;
}

//! @brief ... set compressive strength
//! @param rCompressiveStrength...  compressive strength
void NuTo::DamageViscoPlasticityEngineeringStress::SetCompressiveStrength(double rCompressiveStrength)
{
    this->CheckCompressiveStrength(rCompressiveStrength);
    this->mCompressiveStrength = rCompressiveStrength;
    this->SetParametersValid();
}

//! @brief ... get biaxial compressive strength
//! @return ... biaxial compressive strength
double NuTo::DamageViscoPlasticityEngineeringStress::GetBiaxialCompressiveStrength() const
{
    return mBiaxialCompressiveStrength;
}

//! @brief ... set biaxial compressive strength
//! @param rBiaxialCompressiveStrength...  biaxial compressive strength
void NuTo::DamageViscoPlasticityEngineeringStress::SetBiaxialCompressiveStrength(double rBiaxialCompressiveStrength)
{
    this->CheckBiaxialCompressiveStrength(rBiaxialCompressiveStrength);
    this->mBiaxialCompressiveStrength = rBiaxialCompressiveStrength;
    this->SetParametersValid();
}

///////////////////////////////////////////////////////////////////////////

//! @brief ... get type of constitutive relationship
//! @return ... type of constitutive relationship
//! @sa eConstitutiveType
NuTo::Constitutive::eConstitutiveType NuTo::DamageViscoPlasticityEngineeringStress::GetType() const
{
    return NuTo::Constitutive::LINEAR_ELASTIC;
}

//! @brief ... check compatibility between element type and type of constitutive relationship
//! @param rElementType ... element type
//! @return ... <B>true</B> if the element is compatible with the constitutive relationship, <B>false</B> otherwise.
bool NuTo::DamageViscoPlasticityEngineeringStress::CheckElementCompatibility(NuTo::Element::eElementType rElementType) const
{
    switch (rElementType)
    {
    case NuTo::Element::BRICK8N:
        return true;
    case NuTo::Element::PLANE2D10N:
        return true;
    case NuTo::Element::PLANE2D3N:
        return true;
    case NuTo::Element::PLANE2D4N:
        return true;
    case NuTo::Element::PLANE2D4NSPECTRALORDER2:
        return true;
    case NuTo::Element::PLANE2D4NSPECTRALORDER3:
        return true;
    case NuTo::Element::PLANE2D4NSPECTRALORDER4:
        return true;
    case NuTo::Element::PLANE2D6N:
        return true;
    case NuTo::Element::TETRAHEDRON4N:
        return true;
    case NuTo::Element::TETRAHEDRON10N:
        return true;
    case NuTo::Element::TRUSS1D2N:
        return true;
    case NuTo::Element::TRUSS1D3N:
        return true;
    default:
        return false;
    }
}

//! @brief ... check if density is positive
//! @param rRho ... density
void NuTo::DamageViscoPlasticityEngineeringStress::CheckDensity(double rRho) const
{
    if (rRho < 0.0)
    {
        throw NuTo::MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::CheckDensity] The density must be a positive value.");
    }
}

//! @brief ... check if Young's modulus is positive
//! @param rE ... Young's modulus
void NuTo::DamageViscoPlasticityEngineeringStress::CheckYoungsModulus(double rE) const
{
    if (rE < 0.0)
    {
        throw NuTo::MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::CheckYoungsModulus] The Young's modulus must be a non-negative value.");
    }
}

//! @brief ... check if Poisson's ratio is valid \f$ (-1.0 < \nu < 0.5) \f$
//! @param rE ... Poisson's ratio
void NuTo::DamageViscoPlasticityEngineeringStress::CheckPoissonsRatio(double rNu) const
{
    if (rNu <= -1.0)
    {
        throw NuTo::MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::CheckPoissonsRatio] Poisson's ratio must be greater or equal to -1.0.");
    }
    if (rNu >= 0.5)
    {
        throw NuTo::MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::CheckPoissonsRatio] Poisson's ratio must be smaller or equal to 0.5.");
    }
}

//! @brief ... check thermal expansion coefficient
//! @param rAlpha ... thermal expansion coefficient
void NuTo::DamageViscoPlasticityEngineeringStress::CheckThermalExpansionCoefficient(double rAlpha) const
{
}

//! @brief ... check if tensile strength is positive
//! @param rTensileStrength ... nonlocal radius
void NuTo::DamageViscoPlasticityEngineeringStress::CheckTensileStrength(double rTensileStrength) const
{
    if (rTensileStrength <= 0.0)
    {
        throw NuTo::MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::CheckTensileStrength] The tensile strength must be a positive value.");
    }
}

//! @brief ... check if compressive strength is positive
//! @param rRadius ... compressive strength
void NuTo::DamageViscoPlasticityEngineeringStress::CheckCompressiveStrength(double rCompressiveStrength) const
{
    if (rCompressiveStrength <= 0.0)
    {
        throw NuTo::MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::CheckCompressiveStrength] The compressive strength must be a positive value.");
    }
}

//! @brief ... check if biaxial compressive strength is positive
//! @param rBiaxialCompressiveStrength ... biaxial compressive strength
void NuTo::DamageViscoPlasticityEngineeringStress::CheckBiaxialCompressiveStrength(double rBiaxialCompressiveStrength) const
{
    if (rBiaxialCompressiveStrength <= 0.0)
    {
        throw NuTo::MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::CheckBiaxialCompressiveStrength] The biaxial compressive strength must be a positive value.");
    }
    if (rBiaxialCompressiveStrength <= mCompressiveStrength)
    {
        throw NuTo::MechanicsException("[NuTo::DamageViscoPlasticityEngineeringStress::CheckBiaxialCompressiveStrength] The biaxial compressive strength must be higher than the uniaxial compressive strength.");
    }
}

//! @brief ... print information about the object
//! @param rVerboseLevel ... verbosity of the information
void NuTo::DamageViscoPlasticityEngineeringStress::Info(unsigned short rVerboseLevel, Logger& rLogger) const
{
    this->ConstitutiveBase::Info(rVerboseLevel, rLogger);
    rLogger << "    Young's modulus               : " << this->mE << "\n";
    rLogger << "    Poisson's ratio               : " << this->mNu << "\n";
    rLogger << "    Density                       : " << this->mRho << "\n";
    rLogger << "    thermal expansion coefficient : " << this->mThermalExpansionCoefficient << "\n";
    rLogger << "    tensile strength              : " << this->mTensileStrength << "\n";
    rLogger << "    compressive strength          : " << this->mCompressiveStrength << "\n";
    rLogger << "    biaxial compressive strength  : " << this->mBiaxialCompressiveStrength << "\n";
}

// check parameters
void NuTo::DamageViscoPlasticityEngineeringStress::CheckParameters()const
{
    this->CheckYoungsModulus(this->mE);
    this->CheckPoissonsRatio(this->mNu);
    this->CheckDensity(this->mRho);
    this->CheckThermalExpansionCoefficient(this->mThermalExpansionCoefficient);
}

