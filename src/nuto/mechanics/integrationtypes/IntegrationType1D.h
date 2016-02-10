// $Id$
#ifndef IntegrationType1D_H
#define IntegrationType1D_H

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif //ENABLE_SERIALIZATION

#include "nuto/mechanics/integrationtypes/IntegrationTypeBase.h"

namespace NuTo
{
//! @author Jörg F. Unger, ISM
//! @date November 2009
//! @brief ... abstract class for all integration types in 1D (e.g truss elements or 2Dinterface elements)
class IntegrationType1D : public IntegrationTypeBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief constructor
    IntegrationType1D() {};

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION

    //! @brief returns the dimension of the integration type
    //! @return dimension = 1, 2 or 3
    int GetCoordinateDimension()const
    {
        return 1;
    }

    //! @brief ... check compatibility between element type and integration type
    //! @param rElementType ... element type (enum is defined in ElementBase, but forward declaration of enums not yet possible->int)
    //! @return ... <B>true</B> if the element is compatible with the constitutive relationship, <B>false</B> otherwise.
    virtual bool CheckElementCompatibility(NuTo::Element::eElementType rElementType) const;

protected:
};
} // namespace nuto

#endif //IntegrationType1D_H
