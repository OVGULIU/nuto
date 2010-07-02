// $Id: ConstitutiveTangentLocal.h 102 2009-11-11 10:47:23Z eckardt4 $

#ifndef CONSTITUTIVETANGENTLOCAL_1x1_H
#define CONSTITUTIVETANGENTLOCAL_1x1_H

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/array.hpp>
#endif // ENABLE_SERIALIZATION

#include "nuto/mechanics/constitutive/ConstitutiveTangentBase.h"
#include "nuto/mechanics/MechanicsException.h"

namespace NuTo
{
class LinearElastic;
class ConstitutiveMisesPlasticity;
//! @brief ... tangent matrix for local constitutive formulations
//! @author Jörg F. Unger, ISM
//! @date November 2009
class ConstitutiveTangentLocal1x1: public NuTo::ConstitutiveTangentBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION
    friend class LinearElastic;
    friend class ConstitutiveMisesPlasticity;
    friend class NonlocalDamagePlasticity;

public:
    //! @brief ... constructor
    ConstitutiveTangentLocal1x1();

    //! @brief ... destructor
    ~ConstitutiveTangentLocal1x1();

    //! @brief ... get the number of rows of the tangent matrix
    //! @return ... number of rows
    unsigned int GetNumberOfRows() const;

    //! @brief ... get the number of columns of the tangent matrix
    //! @return ... number of columns
    unsigned int GetNumberOfColumns() const;

    //! @brief ... get the tangent matrix
    //! @brief ... pointer to the tangent matrix (column major storage)
    const double* GetData() const;

    //! @brief reinterpret as ConstitutiveTangentNonlocal3x3, otherwise throw an exception
    ConstitutiveTangentNonlocal3x3* AsConstitutiveTangentNonlocal3x3()
	{
         throw MechanicsException("[NuTo::ConstitutiveTangentLocal1x1::AsConstitutiveTangentNonlocal3x3] data types can not be cast.");
	}

    //! @brief reinterpret as ConstitutiveTangentLocal1x1, otherwise throw an exception
    ConstitutiveTangentLocal1x1* AsConstitutiveTangentLocal1x1()
	{
         return this;
	}

    //! @brief reinterpret as ConstitutiveTangentLocal3x3, otherwise throw an exception
    ConstitutiveTangentLocal3x3* AsConstitutiveTangentLocal3x3()
	{
         throw MechanicsException("[NuTo::ConstitutiveTangentLocal1x1::AsConstitutiveTangentLocal3x3] data types can not be cast.");
	}

    //! @brief reinterpret as ConstitutiveTangentLocal6x6, otherwise throw an exception
    ConstitutiveTangentLocal6x6* AsConstitutiveTangentLocal6x6()
	{
         throw MechanicsException("[NuTo::ConstitutiveTangentLocal1x1::AsConstitutiveTangentLocal6x6] data types can not be cast.");
	}

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstitutiveTangentBase)
           & BOOST_SERIALIZATION_NVP(mTangent);
    }
#endif // ENABLE_SERIALIZATION
private:
    //! @brief ... tangent matrix
    double mTangent;
};

}

#endif // CONSTITUTIVETANGENTLOCAL_1x1_H
