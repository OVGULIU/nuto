#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "math/CustomBoostSerializationExtensions.h"
#endif // ENABLE_SERIALIZATION

#include "mechanics/interpolationtypes/InterpolationBaseIGA.h"
#include "mechanics/integrationtypes/IntegrationTypeBase.h"
#include "mechanics/nodes/NodeEnum.h"

using namespace NuTo;

InterpolationBaseIGA::InterpolationBaseIGA(Node::eDof rDofType, Interpolation::eTypeOrder rTypeOrder, int rDimension)
    : InterpolationBase::InterpolationBase(rDofType, rTypeOrder, rDimension)
{
}


void InterpolationBaseIGA::Initialize()
{
    mNumNodes = CalculateNumNodes();
    mNumDofs = mNumNodes * Node::GetNumComponents(mDofType, mDimension);
}

#ifdef ENABLE_SERIALIZATION
InterpolationBaseIGA::InterpolationBaseIGA()
    :
{
}

template void InterpolationBaseIGA::serialize(boost::archive::binary_oarchive& ar, const unsigned int version);
template void InterpolationBaseIGA::serialize(boost::archive::xml_oarchive& ar, const unsigned int version);
template void InterpolationBaseIGA::serialize(boost::archive::text_oarchive& ar, const unsigned int version);
template void InterpolationBaseIGA::serialize(boost::archive::binary_iarchive& ar, const unsigned int version);
template void InterpolationBaseIGA::serialize(boost::archive::xml_iarchive& ar, const unsigned int version);
template void InterpolationBaseIGA::serialize(boost::archive::text_iarchive& ar, const unsigned int version);
template <class Archive>
void InterpolationBaseIGA::serialize(Archive& ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize InterpolationBaseIGA" << std::endl;
#endif
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(InterpolationBase);
#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize InterpolationBaseIGA" << std::endl;
#endif
}
BOOST_CLASS_EXPORT_IMPLEMENT(InterpolationBaseIGA)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(InterpolationBaseIGA)
#endif // ENABLE_SERIALIZATION
