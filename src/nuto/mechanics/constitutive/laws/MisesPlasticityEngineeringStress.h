#pragma once

#include "nuto/mechanics/constitutive/ConstitutiveBase.h"
#include "nuto/mechanics/constitutive/ConstitutiveEnum.h"
#include "nuto/mechanics/elements/ElementEnum.h"

#include "nuto/mechanics/constitutive/staticData/Leaf.h"
#include "nuto/mechanics/constitutive/staticData/DataMisesPlasticity.h"

namespace NuTo
{
template <int TDim> class EngineeringStrain;
class Logger;
//! @brief Mises plasticity with isotropic and kinematic hardening.
class MisesPlasticityEngineeringStress: public ConstitutiveBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION
public:
    MisesPlasticityEngineeringStress();

    //! @brief ... determines the constitutive inputs needed to evaluate the constitutive outputs
    //! @param rConstitutiveOutput ... desired constitutive outputs
    //! @param rInterpolationType ... interpolation type to determine additional inputs
    //! @return constitutive inputs needed for the evaluation
    ConstitutiveInputMap GetConstitutiveInputs(const ConstitutiveOutputMap& rConstitutiveOutput,
            const InterpolationType& rInterpolationType) const override;

    //! @brief Evaluate the constitutive relation in 1D
    //! @param rConstitutiveInput Input to the constitutive law (strain, temp gradient etc.).
    //! @param rConstitutiveOutput Output to the constitutive law (stress, stiffness, heat flux etc.).
    //! @param staticData Pointer to the history data.
    virtual NuTo::Error::eError Evaluate1D(
            const ConstitutiveInputMap& rConstitutiveInput,
            const ConstitutiveOutputMap& rConstitutiveOutput,
            Constitutive::StaticData::Component* staticData) override;

    //! @brief Evaluate the constitutive relation in 2D
    //! @param rConstitutiveInput Input to the constitutive law (strain, temp gradient etc.).
    //! @param rConstitutiveOutput Output to the constitutive law (stress, stiffness, heat flux etc.).
    //! @param staticData Pointer to the history data.
    virtual NuTo::Error::eError Evaluate2D(
            const ConstitutiveInputMap& rConstitutiveInput,
            const ConstitutiveOutputMap& rConstitutiveOutput,
            Constitutive::StaticData::Component* staticData) override;

    //! @brief Evaluate the constitutive relation in 3D
    //! @param rConstitutiveInput Input to the constitutive law (strain, temp gradient etc.).
    //! @param rConstitutiveOutput Output to the constitutive law (stress, stiffness, heat flux etc.).
    //! @param staticData Pointer to the history data.
    virtual NuTo::Error::eError Evaluate3D(
            const ConstitutiveInputMap& rConstitutiveInput,
            const ConstitutiveOutputMap& rConstitutiveOutput,
            Constitutive::StaticData::Component* staticData) override;

    //! @brief ... create new static data object for an integration point
    //! @return ... pointer to static data object
    Constitutive::StaticData::Component* AllocateStaticData1D(
            const ElementBase* rElement) const override;

    //! @brief ... create new static data object for an integration point
    //! @return ... pointer to static data object
    Constitutive::StaticData::Component* AllocateStaticData2D(
            const ElementBase* rElement) const override;

    //! @brief ... create new static data object for an integration point
    //! @return ... pointer to static data object
    Constitutive::StaticData::Component* AllocateStaticData3D(
            const ElementBase* rElement) const override;

    //! @brief ... determines which submatrices of a multi-doftype problem can be solved by the constitutive law
    //! @param rDofRow ... row dof
    //! @param rDofCol ... column dof
    //! @param rTimeDerivative ... time derivative
    virtual bool CheckDofCombinationComputable(Node::eDof rDofRow,
                                                Node::eDof rDofCol,
                                                int rTimeDerivative) const override;


    //! @brief Performs the return mapping procedure in 2D.
    //! @param rEngineeringStrain Engineering strain.
    //! @param rNewStress New stress. If a `nullptr` is given, no values are written.
    //! @param rNewTangent New tangent matrix. If a `nullptr` is given, no values are written.
    //! @param rNewStaticData New static data. If a `nullptr` is given, no values are written.
    NuTo::Error::eError ReturnMapping2D(Constitutive::StaticData::DataMisesPlasticity<3>& oldStaticData,
            const EngineeringStrain<2>& rEngineeringStrain,
            ConstitutiveIOBase* rNewStress,
            ConstitutiveIOBase* rNewTangent,
            Constitutive::StaticData::DataMisesPlasticity<3>* rNewStaticData) const;

    //! @brief Performs the return mapping procedure in 3D.
    //! @param rEngineeringStrain Engineering strain.
    //! @param rNewStress New stress. If a `nullptr` is given, no values are written.
    //! @param rNewTangent New tangent matrix. If a `nullptr` is given, no values are written.
    //! @param rNewStaticData New static data. If a `nullptr` is given, no values are written.
    NuTo::Error::eError ReturnMapping3D(Constitutive::StaticData::DataMisesPlasticity<3>& oldStaticData,
    		const EngineeringStrain<3>& rEngineeringStrain,
    		ConstitutiveIOBase* rNewStress,
    		ConstitutiveIOBase* rNewTangent,
    		Constitutive::StaticData::DataMisesPlasticity<3>* rNewStaticData) const;

    // parameters /////////////////////////////////////////////////////////////

    //! @brief ... gets a parameter of the constitutive law which is selected by an enum
    //! @param rIdentifier ... Enum to identify the requested parameter
    //! @return ... value of the requested variable
    virtual double GetParameterDouble(Constitutive::eConstitutiveParameter rIdentifier) const override;

    //! @brief ... sets a parameter of the constitutive law which is selected by an enum
    //! @param rIdentifier ... Enum to identify the requested parameter
    //! @param rValue ... new value for requested variable
    virtual void SetParameterDouble(Constitutive::eConstitutiveParameter rIdentifier, double rValue) override;

    //! @brief ... get yield strength for multilinear response
    //! @return ... first column: equivalent plastic strain
    //! @return ... second column: corresponding yield strength
    NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> GetYieldStrength() const override;

    //! @brief ... add yield strength
    //! @param rEpsilon ...  equivalent plastic strain
    //! @param rSigma ...  yield strength
    void AddYieldStrength(double rEpsilon, double rSigma) override;

    //! @brief ... get hardening modulus for multilinear response
    //! @return ... first column: equivalent plastic strain
    //! @return ... second column: corresponding hardening modulus
    NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> GetHardeningModulus() const  override;

    //! @brief ... add hardening modulus
    //! @param rEpsilon ...  equivalent plastic strain
    //! @param rSigma ...  hardening modulus
    void AddHardeningModulus(double rEpsilon, double rSigma) override;

    //! @brief ... get dimension of the constitutive relationship
    //! @return ... dimension of the constitutive relationship (1, 2 or 3)
    int GetGlobalDimension() const;

    //! @brief ... get type of constitutive relationship
    //! @return ... type of constitutive relationship
    //! @sa eConstitutiveType
    Constitutive::eConstitutiveType GetType() const  override;

    //! @brief ... check parameters of the constitutive relationship
    void CheckParameters()const;

    //! @brief ... check compatibility between element type and type of constitutive relationship
    //! @param rElementType ... element type
    //! @return ... <B>true</B> if the element is compatible with the constitutive relationship, <B>false</B> otherwise.
    bool CheckElementCompatibility(Element::eElementType rElementType) const;

    //! @brief ... print information about the object
    //! @param rVerboseLevel ... verbosity of the information
    //! @param rLogger stream for the output
    void Info(unsigned short rVerboseLevel, Logger& rLogger) const;

    //! @brief ... returns true, if a material model has tmp static data (which has to be updated before stress or stiffness are calculated)
    //! @return ... see brief explanation
    virtual bool HaveTmpStaticData() const
    {
    	return false;
    }

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION

protected:
    //! @brief ... Young's modulus \f$ E \f$
    double mE;

    //! @brief ... Poisson's ratio \f$ \nu \f$
    double mNu;

    //! @brief ... density
    double mRho;

    //! @brief ... equivalent strain with corresponding yield strength
    std::vector<std::pair<double, double> > mSigma;

    //! @brief ... equivalent strain with hardening modulus
    std::vector<std::pair<double, double> > mH;

    //! @brief ... check yield strength is positive
    //! @param rSigma ... yield strength
    void CheckYieldStrength(std::vector<std::pair<double, double> > rSigma) const;

    //! @brief ... check hardening modulus
    //! @param rH ... hardening modulus
    void CheckHardeningModulus(std::vector<std::pair<double, double> > rH) const;

    //! @brief ... calculates for a given equivalent plastic strain the radius of the yield surface
    //! @param ... rEpsilonPEq equivalent plastic strain
    //! @param ... rDSigmaDEpsilonP derivative of the yield strength with respect to the plastic strains (return value)
    //! @return ... yield strength (radius of the yield surface)
    double GetYieldStrength(double rEpsilonPEq, double& rDSigmaDEpsilonP)const;

    //! @brief ... calculates for a given equivalent plastic strain the hardening modulus
    //! @param ... rEpsilonPEq equivalent plastic strain
    //! @param ... rDSigmaDEpsilonP derivative of the hardening modulus with respect to the plastic strains (return value)
    //! @return ... hardening modulus
    double GetHardeningModulus(double rEpsilonPEq, double& rDHDEpsilonP)const;
};
}  //namespace NuTo
#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(NuTo::MisesPlasticityEngineeringStress)
#endif // ENABLE_SERIALIZATION
