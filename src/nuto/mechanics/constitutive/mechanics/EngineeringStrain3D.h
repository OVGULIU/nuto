// $Id: EngineeringStrain3D.h $

#ifndef ENGINEERINGSTRAIN3D_H
#define ENGINEERINGSTRAIN3D_H

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#endif // ENABLE_SERIALIZATION

#include "nuto/mechanics/constitutive/ConstitutiveOutputBase.h"

namespace NuTo
{
class DeformationGradient1D;
class DeformationGradient2D;
class DeformationGradient3D;
class LinearElastic;
class LinearElasticEngineeringStress;
class ConstitutiveMisesPlasticity;
class ConstitutiveEngineeringStressStrain;

//! @brief ... three-dimensional deformation gradient
//! @author Jörg F. Unger, ISM
//! @date November 2009
class EngineeringStrain3D: public ConstitutiveOutputBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION
    friend class LinearElastic;
    friend class LinearElasticEngineeringStress;
    friend class ConstitutiveMisesPlasticity;
    friend class ConstitutiveEngineeringStressStrain;
    friend class NonlocalDamagePlasticity;
    friend class Multiscale;
    friend class DeformationGradient1D;
    friend class DeformationGradient2D;
    friend class DeformationGradient3D;
public:
    //! @brief ... constructor
    //! @param pStructure ... structure
    //! @param pElement ... element
    //! @param pIntegrationPoint ... integration point
    EngineeringStrain3D();

    //! @brief ... copy constructor
    EngineeringStrain3D(const DeformationGradient3D& rDeformationGradient);

    //! @brief ... get number of strain components
    //! @return ... number of strain components
    unsigned int GetNumberOfComponents() const;

    //! @brief ... get Engineering Strain
    //! @return ... Engineering Strain (exx,eyy,ezz,gxy,gyz,gzx)
    //! @sa mDeformationGradient
    const double* GetData() const;

    //! @brief ... return engineeringStrain
    EngineeringStrain3D& GetEngineeringStrain3D()
    {
    	return *this;
    }

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION
private:
    //! @brief ... engineering strain
    //! The engineering strain is stored as :
    //! (exx,eyy,ezz,gxy,gyz,gzx)
    double mEngineeringStrain[6];
};

}

#endif // ENGINEERINGSTRAIN3D_H
