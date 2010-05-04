// $ld: $ 
#ifndef VISUALIZECOMPONENTNONLOCALWEIGHT_H_
#define VISUALIZECOMPONENTNONLOCALWEIGHT_H_

#include <boost/serialization/vector.hpp>

#include "nuto/visualize/VisualizeComponentBase.h"

namespace NuTo
{
class ElementWithDataBase;

//! @author Joerg F. Unger
//! @date Apr 27, 2010
//! @brief visualize the nonlocal weights for integration point mIp in element mElement
class VisualizeComponentNonlocalWeight : public VisualizeComponentBase
{
public:
	VisualizeComponentNonlocalWeight();

    void SetElementIp(int rElementId, int rIp);

    std::string GetComponentName()const;

    int GetElementId()const;

    int GetIp()const;

    inline NuTo::VisualizeBase::eVisualizeWhat GetComponentEnum()const
    {
    	return NuTo::VisualizeBase::NONLOCAL_WEIGHT;
    }

protected:
    int mElementId;
    int	 mIp;
};
}
#endif /* VISUALIZECOMPONENTNONLOCALWEIGHT_H_ */
