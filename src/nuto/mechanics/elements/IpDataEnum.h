// $ld: $ 
#ifndef IPDATAENUM_H_
#define IPDATAENUM_H_

namespace NuTo
{
namespace IpData
{
enum eIpDataType
{
    NOIPDATA,            //!< no additional ip data
    STATICDATA,          //!< static data
    STATICDATANONLOCAL   //!< nonlocal and static data
};

//! @brief covers all ip data (not only static data) that is dependent on the current iteration state
//! @brief this is mainly used in Get routines for visualization purposes
enum eIpStaticDataType
{
    ENGINEERING_STRAIN,        //!< engineering strain
    ENGINEERING_STRESS,        //!< engineering stress
    DAMAGE,                    //!< isotropic damage variable
    ENGINEERING_PLASTIC_STRAIN, //!> plastic strain
    ELASTIC_ENERGY,            //!> elastic energy
    TOTAL_ENERGY               //!> total (elastic + inelastic energy
};

}
}
#endif /* IPDATAENUM_H_ */
