// $Id$ 
#ifndef INTEGRATIONTYPEENUM_H_
#define INTEGRATIONTYPEENUM_H_

namespace NuTo
{
namespace IntegrationType
{
//! @author Joerg F. Unger
//! @date May 2, 2010
//! @brief ...

enum eIntegrationType
{
    IntegrationType1D2NGauss1Ip=0,
    IntegrationType1D2NGauss2Ip,
    IntegrationType1D2NBoundaryGauss3Ip,
    IntegrationType1D2NGauss3Ip,
    IntegrationType1D2NLobatto3Ip,
    IntegrationType1D2NLobatto4Ip,
    IntegrationType1D2NLobatto5Ip,
    IntegrationType2D3NGauss13Ip,
    IntegrationType2D3NGauss1Ip,
    IntegrationType2D3NLattice3Ip,
    IntegrationType2D3NGauss3Ip,
    IntegrationType2D4NGauss1Ip,
    IntegrationType2D4NGauss4Ip,
    IntegrationType2D4NLobatto9Ip,
    IntegrationType2D4NLobatto16Ip,
    IntegrationType2D4NLobatto25Ip,
    IntegrationType3D4NGauss1Ip,
    IntegrationType3D4NGauss4Ip,
    IntegrationType3D8NGauss1Ip,
    IntegrationType3D8NGauss2x2x2Ip,
    NumIntegrationTypes
};
}
}
#endif /* INTEGRATIONTYPEENUM_H_ */

