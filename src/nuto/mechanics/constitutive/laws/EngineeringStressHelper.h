#pragma once

#include "nuto/mechanics/constitutive/inputoutput/EngineeringStrain.h"
#include "nuto/mechanics/constitutive/ConstitutiveEnum.h"
#include "nuto/mechanics/interpolationtypes/InterpolationType.h"

namespace NuTo
{

class EngineeringStressHelper
{
public:
    //! @brief calculate coefficients of the PLANE_STRESS 2D material matrix
    //! @param rE ... Young's modulus
    //! @param rNu ... Poisson's ratio
    //! @return tuple <C11, C12, C33>
    static std::tuple<double, double, double> CalculateCoefficients2DPlaneStress(double mE, double mNu);

    //! @brief calculate coefficients of the 3D material matrix
    //! @param rE ... Young's modulus
    //! @param rNu ... Poisson's ratio
    //! @return tuple <C11, C12, C33>
    static std::tuple<double, double, double> CalculateCoefficients3D(double mE, double mNu);
};
} /* namespace NuTo */

