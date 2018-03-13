#include "BoostUnitTest.h"

#include "math/shapes/Tetrahedron.h"
#include "mechanics/integrationtypes/IntegrationTypeBase.h"
#include "mechanics/integrationtypes/IntegrationCompanion.h"

#include <vector>
#include <iostream>

//!@brief Integrates function f by quadrature
//!
//! The integration domain is given by the integration type.
//! @param f function to integrate
//! @param intType integrationtype to provide weights and integration points
//! @result computed integral: a scalar value
double integrate(std::function<double(Eigen::VectorXd)> f, NuTo::IntegrationTypeBase& intType)
{
    double result = 0.;
    for (int i = 0; i < intType.GetNumIntegrationPoints(); i++)
    {
        double y = f(intType.GetLocalIntegrationPointCoordinates(i));
        double w = intType.GetIntegrationPointWeight(i);
        result += w * y;
    }
    return (result);
}

//!@brief Exact values of integrating a monomial over a unit tetrahedron
//!
//! The caluclations are the result of a sympy session (computer algebra package for python)
//! with the following code
//!
//! \code{.py}
//!
//! from sympy import *
//!
//! x,y,z = symbols('x y z')
//! order = 21
//!
//! for n in range(order+1):
//!    out = ''
//!    for i in range(n+1):
//!        for j in range(n+1-i):
//!            term = x**i * y**j * z**(n-i-j)
//!            intX = term.integrate(x)
//!            result = intX.subs(x,1-y-z) - intX.subs(x,0)
//!            intY = result.integrate(y)
//!            result = intY.subs(y,1-z) - intY.subs(y,0)
//!            intZ = result.integrate(z)
//!            result = intZ.subs(z,1) - intZ.subs(z,0)
//!            out += str(result) + ', '
//!    print out
//!
//! \endcode
//!
//! @param order, order of the monomial example: x^3 * y^2 has order 5
//! @param indx Index to specify which monomial of the many of order
//! order is meant.
double ExactIntegralMonomialTetrahedron(int order, int indx)
{
    std::vector<std::vector<double>> expectedResult = {
            {1. / 6},
            {1. / 24, 1. / 24, 1. / 24},
            {1. / 60, 1. / 120, 1. / 60, 1. / 120, 1. / 120, 1. / 60},
            {1. / 120, 1. / 360, 1. / 360, 1. / 120, 1. / 360, 1. / 720, 1. / 360, 1. / 360, 1. / 360, 1. / 120},
            {1. / 210, 1. / 840, 1. / 1260, 1. / 840, 1. / 210, 1. / 840, 1. / 2520, 1. / 2520, 1. / 840, 1. / 1260,
             1. / 2520, 1. / 1260, 1. / 840, 1. / 840, 1. / 210},
            {1. / 336,  1. / 1680,  1. / 3360, 1. / 3360, 1. / 1680, 1. / 336,   1. / 1680,
             1. / 6720, 1. / 10080, 1. / 6720, 1. / 1680, 1. / 3360, 1. / 10080, 1. / 10080,
             1. / 3360, 1. / 3360,  1. / 6720, 1. / 3360, 1. / 1680, 1. / 1680,  1. / 336},
            {1. / 504,   1. / 3024,  1. / 7560,  1. / 10080, 1. / 7560,  1. / 3024,  1. / 504,
             1. / 3024,  1. / 15120, 1. / 30240, 1. / 30240, 1. / 15120, 1. / 3024,  1. / 7560,
             1. / 30240, 1. / 45360, 1. / 30240, 1. / 7560,  1. / 10080, 1. / 30240, 1. / 30240,
             1. / 10080, 1. / 7560,  1. / 15120, 1. / 7560,  1. / 3024,  1. / 3024,  1. / 504},
            {1. / 720,    1. / 5040,   1. / 15120,  1. / 25200,  1. / 25200, 1. / 15120, 1. / 5040,   1. / 720,
             1. / 5040,   1. / 30240,  1. / 75600,  1. / 100800, 1. / 75600, 1. / 30240, 1. / 5040,   1. / 15120,
             1. / 75600,  1. / 151200, 1. / 151200, 1. / 75600,  1. / 15120, 1. / 25200, 1. / 100800, 1. / 151200,
             1. / 100800, 1. / 25200,  1. / 25200,  1. / 75600,  1. / 75600, 1. / 25200, 1. / 15120,  1. / 30240,
             1. / 15120,  1. / 5040,   1. / 5040,   1. / 720},
            {1. / 990,    1. / 7920,   1. / 27720,  1. / 55440,  1. / 69300,  1. / 55440,  1. / 27720,  1. / 7920,
             1. / 990,    1. / 7920,   1. / 55440,  1. / 166320, 1. / 277200, 1. / 277200, 1. / 166320, 1. / 55440,
             1. / 7920,   1. / 27720,  1. / 166320, 1. / 415800, 1. / 554400, 1. / 415800, 1. / 166320, 1. / 27720,
             1. / 55440,  1. / 277200, 1. / 554400, 1. / 554400, 1. / 277200, 1. / 55440,  1. / 69300,  1. / 277200,
             1. / 415800, 1. / 277200, 1. / 69300,  1. / 55440,  1. / 166320, 1. / 166320, 1. / 55440,  1. / 27720,
             1. / 55440,  1. / 27720,  1. / 7920,   1. / 7920,   1. / 990},
            {1. / 1320,   1. / 11880,  1. / 47520,  1. / 110880, 1. / 166320,  1. / 166320,  1. / 110880,  1. / 47520,
             1. / 11880,  1. / 1320,   1. / 11880,  1. / 95040,  1. / 332640,  1. / 665280,  1. / 831600,  1. / 665280,
             1. / 332640, 1. / 95040,  1. / 11880,  1. / 47520,  1. / 332640,  1. / 997920,  1. / 1663200, 1. / 1663200,
             1. / 997920, 1. / 332640, 1. / 47520,  1. / 110880, 1. / 665280,  1. / 1663200, 1. / 2217600, 1. / 1663200,
             1. / 665280, 1. / 110880, 1. / 166320, 1. / 831600, 1. / 1663200, 1. / 1663200, 1. / 831600,  1. / 166320,
             1. / 166320, 1. / 665280, 1. / 997920, 1. / 665280, 1. / 166320,  1. / 110880,  1. / 332640,  1. / 332640,
             1. / 110880, 1. / 47520,  1. / 95040,  1. / 47520,  1. / 11880,   1. / 11880,   1. / 1320},
            {1. / 1716,    1. / 17160,   1. / 77220,   1. / 205920,  1. / 360360,  1. / 432432,  1. / 360360,
             1. / 205920,  1. / 77220,   1. / 17160,   1. / 1716,    1. / 17160,   1. / 154440,  1. / 617760,
             1. / 1441440, 1. / 2162160, 1. / 2162160, 1. / 1441440, 1. / 617760,  1. / 154440,  1. / 17160,
             1. / 77220,   1. / 617760,  1. / 2162160, 1. / 4324320, 1. / 5405400, 1. / 4324320, 1. / 2162160,
             1. / 617760,  1. / 77220,   1. / 205920,  1. / 1441440, 1. / 4324320, 1. / 7207200, 1. / 7207200,
             1. / 4324320, 1. / 1441440, 1. / 205920,  1. / 360360,  1. / 2162160, 1. / 5405400, 1. / 7207200,
             1. / 5405400, 1. / 2162160, 1. / 360360,  1. / 432432,  1. / 2162160, 1. / 4324320, 1. / 4324320,
             1. / 2162160, 1. / 432432,  1. / 360360,  1. / 1441440, 1. / 2162160, 1. / 1441440, 1. / 360360,
             1. / 205920,  1. / 617760,  1. / 617760,  1. / 205920,  1. / 77220,   1. / 154440,  1. / 77220,
             1. / 17160,   1. / 17160,   1. / 1716},
            {1. / 2184,     1. / 24024,    1. / 120120,   1. / 360360,   1. / 720720,   1. / 1009008,  1. / 1009008,
             1. / 720720,   1. / 360360,   1. / 120120,   1. / 24024,    1. / 2184,     1. / 24024,    1. / 240240,
             1. / 1081080,  1. / 2882880,  1. / 5045040,  1. / 6054048,  1. / 5045040,  1. / 2882880,  1. / 1081080,
             1. / 240240,   1. / 24024,    1. / 120120,   1. / 1081080,  1. / 4324320,  1. / 10090080, 1. / 15135120,
             1. / 15135120, 1. / 10090080, 1. / 4324320,  1. / 1081080,  1. / 120120,   1. / 360360,   1. / 2882880,
             1. / 10090080, 1. / 20180160, 1. / 25225200, 1. / 20180160, 1. / 10090080, 1. / 2882880,  1. / 360360,
             1. / 720720,   1. / 5045040,  1. / 15135120, 1. / 25225200, 1. / 25225200, 1. / 15135120, 1. / 5045040,
             1. / 720720,   1. / 1009008,  1. / 6054048,  1. / 15135120, 1. / 20180160, 1. / 15135120, 1. / 6054048,
             1. / 1009008,  1. / 1009008,  1. / 5045040,  1. / 10090080, 1. / 10090080, 1. / 5045040,  1. / 1009008,
             1. / 720720,   1. / 2882880,  1. / 4324320,  1. / 2882880,  1. / 720720,   1. / 360360,   1. / 1081080,
             1. / 1081080,  1. / 360360,   1. / 120120,   1. / 240240,   1. / 120120,   1. / 24024,    1. / 24024,
             1. / 2184},
            {1. / 2730,     1. / 32760,    1. / 180180,   1. / 600600,   1. / 1351350,  1. / 2162160,  1. / 2522520,
             1. / 2162160,  1. / 1351350,  1. / 600600,   1. / 180180,   1. / 32760,    1. / 2730,     1. / 32760,
             1. / 360360,   1. / 1801800,  1. / 5405400,  1. / 10810800, 1. / 15135120, 1. / 15135120, 1. / 10810800,
             1. / 5405400,  1. / 1801800,  1. / 360360,   1. / 32760,    1. / 180180,   1. / 1801800,  1. / 8108100,
             1. / 21621600, 1. / 37837800, 1. / 45405360, 1. / 37837800, 1. / 21621600, 1. / 8108100,  1. / 1801800,
             1. / 180180,   1. / 600600,   1. / 5405400,  1. / 21621600, 1. / 50450400, 1. / 75675600, 1. / 75675600,
             1. / 50450400, 1. / 21621600, 1. / 5405400,  1. / 600600,   1. / 1351350,  1. / 10810800, 1. / 37837800,
             1. / 75675600, 1. / 94594500, 1. / 75675600, 1. / 37837800, 1. / 10810800, 1. / 1351350,  1. / 2162160,
             1. / 15135120, 1. / 45405360, 1. / 75675600, 1. / 75675600, 1. / 45405360, 1. / 15135120, 1. / 2162160,
             1. / 2522520,  1. / 15135120, 1. / 37837800, 1. / 50450400, 1. / 37837800, 1. / 15135120, 1. / 2522520,
             1. / 2162160,  1. / 10810800, 1. / 21621600, 1. / 21621600, 1. / 10810800, 1. / 2162160,  1. / 1351350,
             1. / 5405400,  1. / 8108100,  1. / 5405400,  1. / 1351350,  1. / 600600,   1. / 1801800,  1. / 1801800,
             1. / 600600,   1. / 180180,   1. / 360360,   1. / 180180,   1. / 32760,    1. / 32760,    1. / 2730},
            {1. / 3360,      1. / 43680,     1. / 262080,    1. / 960960,    1. / 2402400,   1. / 4324320,
             1. / 5765760,   1. / 5765760,   1. / 4324320,   1. / 2402400,   1. / 960960,    1. / 262080,
             1. / 43680,     1. / 3360,      1. / 43680,     1. / 524160,    1. / 2882880,   1. / 9609600,
             1. / 21621600,  1. / 34594560,  1. / 40360320,  1. / 34594560,  1. / 21621600,  1. / 9609600,
             1. / 2882880,   1. / 524160,    1. / 43680,     1. / 262080,    1. / 2882880,   1. / 14414400,
             1. / 43243200,  1. / 86486400,  1. / 121080960, 1. / 121080960, 1. / 86486400,  1. / 43243200,
             1. / 14414400,  1. / 2882880,   1. / 262080,    1. / 960960,    1. / 9609600,   1. / 43243200,
             1. / 115315200, 1. / 201801600, 1. / 242161920, 1. / 201801600, 1. / 115315200, 1. / 43243200,
             1. / 9609600,   1. / 960960,    1. / 2402400,   1. / 21621600,  1. / 86486400,  1. / 201801600,
             1. / 302702400, 1. / 302702400, 1. / 201801600, 1. / 86486400,  1. / 21621600,  1. / 2402400,
             1. / 4324320,   1. / 34594560,  1. / 121080960, 1. / 242161920, 1. / 302702400, 1. / 242161920,
             1. / 121080960, 1. / 34594560,  1. / 4324320,   1. / 5765760,   1. / 40360320,  1. / 121080960,
             1. / 201801600, 1. / 201801600, 1. / 121080960, 1. / 40360320,  1. / 5765760,   1. / 5765760,
             1. / 34594560,  1. / 86486400,  1. / 115315200, 1. / 86486400,  1. / 34594560,  1. / 5765760,
             1. / 4324320,   1. / 21621600,  1. / 43243200,  1. / 43243200,  1. / 21621600,  1. / 4324320,
             1. / 2402400,   1. / 9609600,   1. / 14414400,  1. / 9609600,   1. / 2402400,   1. / 960960,
             1. / 2882880,   1. / 2882880,   1. / 960960,    1. / 262080,    1. / 524160,    1. / 262080,
             1. / 43680,     1. / 43680,     1. / 3360},
            {1. / 4080,      1. / 57120,     1. / 371280,    1. / 1485120,    1. / 4084080,    1. / 8168160,
             1. / 12252240,  1. / 14002560,  1. / 12252240,  1. / 8168160,    1. / 4084080,    1. / 1485120,
             1. / 371280,    1. / 57120,     1. / 4080,      1. / 57120,      1. / 742560,     1. / 4455360,
             1. / 16336320,  1. / 40840800,  1. / 73513440,  1. / 98017920,   1. / 98017920,   1. / 73513440,
             1. / 40840800,  1. / 16336320,  1. / 4455360,   1. / 742560,     1. / 57120,      1. / 371280,
             1. / 4455360,   1. / 24504480,  1. / 81681600,  1. / 183783600,  1. / 294053760,  1. / 343062720,
             1. / 294053760, 1. / 183783600, 1. / 81681600,  1. / 24504480,   1. / 4455360,    1. / 371280,
             1. / 1485120,   1. / 16336320,  1. / 81681600,  1. / 245044800,  1. / 490089600,  1. / 686125440,
             1. / 686125440, 1. / 490089600, 1. / 245044800, 1. / 81681600,   1. / 16336320,   1. / 1485120,
             1. / 4084080,   1. / 40840800,  1. / 183783600, 1. / 490089600,  1. / 857656800,  1. / 1029188160,
             1. / 857656800, 1. / 490089600, 1. / 183783600, 1. / 40840800,   1. / 4084080,    1. / 8168160,
             1. / 73513440,  1. / 294053760, 1. / 686125440, 1. / 1029188160, 1. / 1029188160, 1. / 686125440,
             1. / 294053760, 1. / 73513440,  1. / 8168160,   1. / 12252240,   1. / 98017920,   1. / 343062720,
             1. / 686125440, 1. / 857656800, 1. / 686125440, 1. / 343062720,  1. / 98017920,   1. / 12252240,
             1. / 14002560,  1. / 98017920,  1. / 294053760, 1. / 490089600,  1. / 490089600,  1. / 294053760,
             1. / 98017920,  1. / 14002560,  1. / 12252240,  1. / 73513440,   1. / 183783600,  1. / 245044800,
             1. / 183783600, 1. / 73513440,  1. / 12252240,  1. / 8168160,    1. / 40840800,   1. / 81681600,
             1. / 81681600,  1. / 40840800,  1. / 8168160,   1. / 4084080,    1. / 16336320,   1. / 24504480,
             1. / 16336320,  1. / 4084080,   1. / 1485120,   1. / 4455360,    1. / 4455360,    1. / 1485120,
             1. / 371280,    1. / 742560,    1. / 371280,    1. / 57120,      1. / 57120,      1. / 4080},
            {1. / 4896,       1. / 73440,      1. / 514080,     1. / 2227680,    1. / 6683040,    1. / 14702688,
             1. / 24504480,   1. / 31505760,   1. / 31505760,   1. / 24504480,   1. / 14702688,   1. / 6683040,
             1. / 2227680,    1. / 514080,     1. / 73440,      1. / 4896,       1. / 73440,      1. / 1028160,
             1. / 6683040,    1. / 26732160,   1. / 73513440,   1. / 147026880,  1. / 220540320,  1. / 252046080,
             1. / 220540320,  1. / 147026880,  1. / 73513440,   1. / 26732160,   1. / 6683040,    1. / 1028160,
             1. / 73440,      1. / 514080,     1. / 6683040,    1. / 40098240,   1. / 147026880,  1. / 367567200,
             1. / 661620960,  1. / 882161280,  1. / 882161280,  1. / 661620960,  1. / 367567200,  1. / 147026880,
             1. / 40098240,   1. / 6683040,    1. / 514080,     1. / 2227680,    1. / 26732160,   1. / 147026880,
             1. / 490089600,  1. / 1102701600, 1. / 1764322560, 1. / 2058376320, 1. / 1764322560, 1. / 1102701600,
             1. / 490089600,  1. / 147026880,  1. / 26732160,   1. / 2227680,    1. / 6683040,    1. / 73513440,
             1. / 367567200,  1. / 1102701600, 1. / 2205403200, 1. / 3087564480, 1. / 3087564480, 1. / 2205403200,
             1. / 1102701600, 1. / 367567200,  1. / 73513440,   1. / 6683040,    1. / 14702688,   1. / 147026880,
             1. / 661620960,  1. / 1764322560, 1. / 3087564480, 1. / 3705077376, 1. / 3087564480, 1. / 1764322560,
             1. / 661620960,  1. / 147026880,  1. / 14702688,   1. / 24504480,   1. / 220540320,  1. / 882161280,
             1. / 2058376320, 1. / 3087564480, 1. / 3087564480, 1. / 2058376320, 1. / 882161280,  1. / 220540320,
             1. / 24504480,   1. / 31505760,   1. / 252046080,  1. / 882161280,  1. / 1764322560, 1. / 2205403200,
             1. / 1764322560, 1. / 882161280,  1. / 252046080,  1. / 31505760,   1. / 31505760,   1. / 220540320,
             1. / 661620960,  1. / 1102701600, 1. / 1102701600, 1. / 661620960,  1. / 220540320,  1. / 31505760,
             1. / 24504480,   1. / 147026880,  1. / 367567200,  1. / 490089600,  1. / 367567200,  1. / 147026880,
             1. / 24504480,   1. / 14702688,   1. / 73513440,   1. / 147026880,  1. / 147026880,  1. / 73513440,
             1. / 14702688,   1. / 6683040,    1. / 26732160,   1. / 40098240,   1. / 26732160,   1. / 6683040,
             1. / 2227680,    1. / 6683040,    1. / 6683040,    1. / 2227680,    1. / 514080,     1. / 1028160,
             1. / 514080,     1. / 73440,      1. / 73440,      1. / 4896},
            {1. / 5814,       1. / 93024,      1. / 697680,      1. / 3255840,     1. / 10581480,   1. / 25395552,
             1. / 46558512,   1. / 66512160,   1. / 74826180,    1. / 66512160,    1. / 46558512,   1. / 25395552,
             1. / 10581480,   1. / 3255840,    1. / 697680,      1. / 93024,       1. / 5814,       1. / 93024,
             1. / 1395360,    1. / 9767520,    1. / 42325920,    1. / 126977760,   1. / 279351072,  1. / 465585120,
             1. / 598609440,  1. / 598609440,  1. / 465585120,   1. / 279351072,   1. / 126977760,  1. / 42325920,
             1. / 9767520,    1. / 1395360,    1. / 93024,       1. / 697680,      1. / 9767520,    1. / 63488880,
             1. / 253955520,  1. / 698377680,  1. / 1396755360,  1. / 2095133040,  1. / 2394437760, 1. / 2095133040,
             1. / 1396755360, 1. / 698377680,  1. / 253955520,   1. / 63488880,    1. / 9767520,    1. / 697680,
             1. / 3255840,    1. / 42325920,   1. / 253955520,   1. / 931170240,   1. / 2327925600, 1. / 4190266080,
             1. / 5587021440, 1. / 5587021440, 1. / 4190266080,  1. / 2327925600,  1. / 931170240,  1. / 253955520,
             1. / 42325920,   1. / 3255840,    1. / 10581480,    1. / 126977760,   1. / 698377680,  1. / 2327925600,
             1. / 5237832600, 1. / 8380532160, 1. / 9777287520,  1. / 8380532160,  1. / 5237832600, 1. / 2327925600,
             1. / 698377680,  1. / 126977760,  1. / 10581480,    1. / 25395552,    1. / 279351072,  1. / 1396755360,
             1. / 4190266080, 1. / 8380532160, 1. / 11732745024, 1. / 11732745024, 1. / 8380532160, 1. / 4190266080,
             1. / 1396755360, 1. / 279351072,  1. / 25395552,    1. / 46558512,    1. / 465585120,  1. / 2095133040,
             1. / 5587021440, 1. / 9777287520, 1. / 11732745024, 1. / 9777287520,  1. / 5587021440, 1. / 2095133040,
             1. / 465585120,  1. / 46558512,   1. / 66512160,    1. / 598609440,   1. / 2394437760, 1. / 5587021440,
             1. / 8380532160, 1. / 8380532160, 1. / 5587021440,  1. / 2394437760,  1. / 598609440,  1. / 66512160,
             1. / 74826180,   1. / 598609440,  1. / 2095133040,  1. / 4190266080,  1. / 5237832600, 1. / 4190266080,
             1. / 2095133040, 1. / 598609440,  1. / 74826180,    1. / 66512160,    1. / 465585120,  1. / 1396755360,
             1. / 2327925600, 1. / 2327925600, 1. / 1396755360,  1. / 465585120,   1. / 66512160,   1. / 46558512,
             1. / 279351072,  1. / 698377680,  1. / 931170240,   1. / 698377680,   1. / 279351072,  1. / 46558512,
             1. / 25395552,   1. / 126977760,  1. / 253955520,   1. / 253955520,   1. / 126977760,  1. / 25395552,
             1. / 10581480,   1. / 42325920,   1. / 63488880,    1. / 42325920,    1. / 10581480,   1. / 3255840,
             1. / 9767520,    1. / 9767520,    1. / 3255840,     1. / 697680,      1. / 1395360,    1. / 697680,
             1. / 93024,      1. / 93024,      1. / 5814},
            {1. / 6840,        1. / 116280,      1. / 930240,      1. / 4651200,     1. / 16279200,    1. / 42325920,
             1. / 84651840,    1. / 133024320,   1. / 166280400,   1. / 166280400,   1. / 133024320,   1. / 84651840,
             1. / 42325920,    1. / 16279200,    1. / 4651200,     1. / 930240,      1. / 116280,      1. / 6840,
             1. / 116280,      1. / 1860480,     1. / 13953600,    1. / 65116800,    1. / 211629600,   1. / 507911040,
             1. / 931170240,   1. / 1330243200,  1. / 1496523600,  1. / 1330243200,  1. / 931170240,   1. / 507911040,
             1. / 211629600,   1. / 65116800,    1. / 13953600,    1. / 1860480,     1. / 116280,      1. / 930240,
             1. / 13953600,    1. / 97675200,    1. / 423259200,   1. / 1269777600,  1. / 2793510720,  1. / 4655851200,
             1. / 5986094400,  1. / 5986094400,  1. / 4655851200,  1. / 2793510720,  1. / 1269777600,  1. / 423259200,
             1. / 97675200,    1. / 13953600,    1. / 930240,      1. / 4651200,     1. / 65116800,    1. / 423259200,
             1. / 1693036800,  1. / 4655851200,  1. / 9311702400,  1. / 13967553600, 1. / 15962918400, 1. / 13967553600,
             1. / 9311702400,  1. / 4655851200,  1. / 1693036800,  1. / 423259200,   1. / 65116800,    1. / 4651200,
             1. / 16279200,    1. / 211629600,   1. / 1269777600,  1. / 4655851200,  1. / 11639628000, 1. / 20951330400,
             1. / 27935107200, 1. / 27935107200, 1. / 20951330400, 1. / 11639628000, 1. / 4655851200,  1. / 1269777600,
             1. / 211629600,   1. / 16279200,    1. / 42325920,    1. / 507911040,   1. / 2793510720,  1. / 9311702400,
             1. / 20951330400, 1. / 33522128640, 1. / 39109150080, 1. / 33522128640, 1. / 20951330400, 1. / 9311702400,
             1. / 2793510720,  1. / 507911040,   1. / 42325920,    1. / 84651840,    1. / 931170240,   1. / 4655851200,
             1. / 13967553600, 1. / 27935107200, 1. / 39109150080, 1. / 39109150080, 1. / 27935107200, 1. / 13967553600,
             1. / 4655851200,  1. / 931170240,   1. / 84651840,    1. / 133024320,   1. / 1330243200,  1. / 5986094400,
             1. / 15962918400, 1. / 27935107200, 1. / 33522128640, 1. / 27935107200, 1. / 15962918400, 1. / 5986094400,
             1. / 1330243200,  1. / 133024320,   1. / 166280400,   1. / 1496523600,  1. / 5986094400,  1. / 13967553600,
             1. / 20951330400, 1. / 20951330400, 1. / 13967553600, 1. / 5986094400,  1. / 1496523600,  1. / 166280400,
             1. / 166280400,   1. / 1330243200,  1. / 4655851200,  1. / 9311702400,  1. / 11639628000, 1. / 9311702400,
             1. / 4655851200,  1. / 1330243200,  1. / 166280400,   1. / 133024320,   1. / 931170240,   1. / 2793510720,
             1. / 4655851200,  1. / 4655851200,  1. / 2793510720,  1. / 931170240,   1. / 133024320,   1. / 84651840,
             1. / 507911040,   1. / 1269777600,  1. / 1693036800,  1. / 1269777600,  1. / 507911040,   1. / 84651840,
             1. / 42325920,    1. / 211629600,   1. / 423259200,   1. / 423259200,   1. / 211629600,   1. / 42325920,
             1. / 16279200,    1. / 65116800,    1. / 97675200,    1. / 65116800,    1. / 16279200,    1. / 4651200,
             1. / 13953600,    1. / 13953600,    1. / 4651200,     1. / 930240,      1. / 1860480,     1. / 930240,
             1. / 116280,      1. / 116280,      1. / 6840},
            {1. / 7980,         1. / 143640,       1. / 1220940,      1. / 6511680,      1. / 24418800,
             1. / 68372640,     1. / 148140720,    1. / 253955520,    1. / 349188840,    1. / 387987600,
             1. / 349188840,    1. / 253955520,    1. / 148140720,    1. / 68372640,     1. / 24418800,
             1. / 6511680,      1. / 1220940,      1. / 143640,       1. / 7980,         1. / 143640,
             1. / 2441880,      1. / 19535040,     1. / 97675200,     1. / 341863200,    1. / 888844320,
             1. / 1777688640,   1. / 2793510720,   1. / 3491888400,   1. / 3491888400,   1. / 2793510720,
             1. / 1777688640,   1. / 888844320,    1. / 341863200,    1. / 97675200,     1. / 19535040,
             1. / 2441880,      1. / 143640,       1. / 1220940,      1. / 19535040,     1. / 146512800,
             1. / 683726400,    1. / 2222110800,   1. / 5333065920,   1. / 9777287520,   1. / 13967553600,
             1. / 15713497800,  1. / 13967553600,  1. / 9777287520,   1. / 5333065920,   1. / 2222110800,
             1. / 683726400,    1. / 146512800,    1. / 19535040,     1. / 1220940,      1. / 6511680,
             1. / 97675200,     1. / 683726400,    1. / 2962814400,   1. / 8888443200,   1. / 19554575040,
             1. / 32590958400,  1. / 41902660800,  1. / 41902660800,  1. / 32590958400,  1. / 19554575040,
             1. / 8888443200,   1. / 2962814400,   1. / 683726400,    1. / 97675200,     1. / 6511680,
             1. / 24418800,     1. / 341863200,    1. / 2222110800,   1. / 8888443200,   1. / 24443218800,
             1. / 48886437600,  1. / 73329656400,  1. / 83805321600,  1. / 73329656400,  1. / 48886437600,
             1. / 24443218800,  1. / 8888443200,   1. / 2222110800,   1. / 341863200,    1. / 24418800,
             1. / 68372640,     1. / 888844320,    1. / 5333065920,   1. / 19554575040,  1. / 48886437600,
             1. / 87995587680,  1. / 117327450240, 1. / 117327450240, 1. / 87995587680,  1. / 48886437600,
             1. / 19554575040,  1. / 5333065920,   1. / 888844320,    1. / 68372640,     1. / 148140720,
             1. / 1777688640,   1. / 9777287520,   1. / 32590958400,  1. / 73329656400,  1. / 117327450240,
             1. / 136882025280, 1. / 117327450240, 1. / 73329656400,  1. / 32590958400,  1. / 9777287520,
             1. / 1777688640,   1. / 148140720,    1. / 253955520,    1. / 2793510720,   1. / 13967553600,
             1. / 41902660800,  1. / 83805321600,  1. / 117327450240, 1. / 117327450240, 1. / 83805321600,
             1. / 41902660800,  1. / 13967553600,  1. / 2793510720,   1. / 253955520,    1. / 349188840,
             1. / 3491888400,   1. / 15713497800,  1. / 41902660800,  1. / 73329656400,  1. / 87995587680,
             1. / 73329656400,  1. / 41902660800,  1. / 15713497800,  1. / 3491888400,   1. / 349188840,
             1. / 387987600,    1. / 3491888400,   1. / 13967553600,  1. / 32590958400,  1. / 48886437600,
             1. / 48886437600,  1. / 32590958400,  1. / 13967553600,  1. / 3491888400,   1. / 387987600,
             1. / 349188840,    1. / 2793510720,   1. / 9777287520,   1. / 19554575040,  1. / 24443218800,
             1. / 19554575040,  1. / 9777287520,   1. / 2793510720,   1. / 349188840,    1. / 253955520,
             1. / 1777688640,   1. / 5333065920,   1. / 8888443200,   1. / 8888443200,   1. / 5333065920,
             1. / 1777688640,   1. / 253955520,    1. / 148140720,    1. / 888844320,    1. / 2222110800,
             1. / 2962814400,   1. / 2222110800,   1. / 888844320,    1. / 148140720,    1. / 68372640,
             1. / 341863200,    1. / 683726400,    1. / 683726400,    1. / 341863200,    1. / 68372640,
             1. / 24418800,     1. / 97675200,     1. / 146512800,    1. / 97675200,     1. / 24418800,
             1. / 6511680,      1. / 19535040,     1. / 19535040,     1. / 6511680,      1. / 1220940,
             1. / 2441880,      1. / 1220940,      1. / 143640,       1. / 143640,       1. / 7980},
            {1. / 9240,         1. / 175560,       1. / 1580040,      1. / 8953560,      1. / 35814240,
             1. / 107442720,    1. / 250699680,    1. / 465585120,    1. / 698377680,    1. / 853572720,
             1. / 853572720,    1. / 698377680,    1. / 465585120,    1. / 250699680,    1. / 107442720,
             1. / 35814240,     1. / 8953560,      1. / 1580040,      1. / 175560,       1. / 9240,
             1. / 175560,       1. / 3160080,      1. / 26860680,     1. / 143256960,    1. / 537213600,
             1. / 1504198080,   1. / 3259095840,   1. / 5587021440,   1. / 7682154480,   1. / 8535727200,
             1. / 7682154480,   1. / 5587021440,   1. / 3259095840,   1. / 1504198080,   1. / 537213600,
             1. / 143256960,    1. / 26860680,     1. / 3160080,      1. / 175560,       1. / 1580040,
             1. / 26860680,     1. / 214885440,    1. / 1074427200,   1. / 3760495200,   1. / 9777287520,
             1. / 19554575040,  1. / 30728617920,  1. / 38410772400,  1. / 38410772400,  1. / 30728617920,
             1. / 19554575040,  1. / 9777287520,   1. / 3760495200,   1. / 1074427200,   1. / 214885440,
             1. / 26860680,     1. / 1580040,      1. / 8953560,      1. / 143256960,    1. / 1074427200,
             1. / 5013993600,   1. / 16295479200,  1. / 39109150080,  1. / 71700108480,  1. / 102428726400,
             1. / 115232317200, 1. / 102428726400, 1. / 71700108480,  1. / 39109150080,  1. / 16295479200,
             1. / 5013993600,   1. / 1074427200,   1. / 143256960,    1. / 8953560,      1. / 35814240,
             1. / 537213600,    1. / 3760495200,   1. / 16295479200,  1. / 48886437600,  1. / 107550162720,
             1. / 179250271200, 1. / 230464634400, 1. / 230464634400, 1. / 179250271200, 1. / 107550162720,
             1. / 48886437600,  1. / 16295479200,  1. / 3760495200,   1. / 537213600,    1. / 35814240,
             1. / 107442720,    1. / 1504198080,   1. / 9777287520,   1. / 39109150080,  1. / 107550162720,
             1. / 215100325440, 1. / 322650488160, 1. / 368743415040, 1. / 322650488160, 1. / 215100325440,
             1. / 107550162720, 1. / 39109150080,  1. / 9777287520,   1. / 1504198080,   1. / 107442720,
             1. / 250699680,    1. / 3259095840,   1. / 19554575040,  1. / 71700108480,  1. / 179250271200,
             1. / 322650488160, 1. / 430200650880, 1. / 430200650880, 1. / 322650488160, 1. / 179250271200,
             1. / 71700108480,  1. / 19554575040,  1. / 3259095840,   1. / 250699680,    1. / 465585120,
             1. / 5587021440,   1. / 30728617920,  1. / 102428726400, 1. / 230464634400, 1. / 368743415040,
             1. / 430200650880, 1. / 368743415040, 1. / 230464634400, 1. / 102428726400, 1. / 30728617920,
             1. / 5587021440,   1. / 465585120,    1. / 698377680,    1. / 7682154480,   1. / 38410772400,
             1. / 115232317200, 1. / 230464634400, 1. / 322650488160, 1. / 322650488160, 1. / 230464634400,
             1. / 115232317200, 1. / 38410772400,  1. / 7682154480,   1. / 698377680,    1. / 853572720,
             1. / 8535727200,   1. / 38410772400,  1. / 102428726400, 1. / 179250271200, 1. / 215100325440,
             1. / 179250271200, 1. / 102428726400, 1. / 38410772400,  1. / 8535727200,   1. / 853572720,
             1. / 853572720,    1. / 7682154480,   1. / 30728617920,  1. / 71700108480,  1. / 107550162720,
             1. / 107550162720, 1. / 71700108480,  1. / 30728617920,  1. / 7682154480,   1. / 853572720,
             1. / 698377680,    1. / 5587021440,   1. / 19554575040,  1. / 39109150080,  1. / 48886437600,
             1. / 39109150080,  1. / 19554575040,  1. / 5587021440,   1. / 698377680,    1. / 465585120,
             1. / 3259095840,   1. / 9777287520,   1. / 16295479200,  1. / 16295479200,  1. / 9777287520,
             1. / 3259095840,   1. / 465585120,    1. / 250699680,    1. / 1504198080,   1. / 3760495200,
             1. / 5013993600,   1. / 3760495200,   1. / 1504198080,   1. / 250699680,    1. / 107442720,
             1. / 537213600,    1. / 1074427200,   1. / 1074427200,   1. / 537213600,    1. / 107442720,
             1. / 35814240,     1. / 143256960,    1. / 214885440,    1. / 143256960,    1. / 35814240,
             1. / 8953560,      1. / 26860680,     1. / 26860680,     1. / 8953560,      1. / 1580040,
             1. / 3160080,      1. / 1580040,      1. / 175560,       1. / 175560,       1. / 9240}};
    return expectedResult[order][indx];
}

void CheckTetrahedronIntegration(int polyOrder, int intTypeOrder)
{
    std::unique_ptr<NuTo::IntegrationTypeBase> intType =
            NuTo::CreateGaussIntegrationType(NuTo::Tetrahedron(), intTypeOrder);
    for (int n = 0; n <= polyOrder; n++)
    {
        int count = 0;
        for (int i = 0; i < n + 1; i++)
        {
            for (int j = 0; j < (n + 1 - i); j++)
            {
                auto f = [n, i, j](Eigen::VectorXd x) {
                    return (std::pow(x[0], i) * std::pow(x[1], j) * std::pow(x[2], n - i - j));
                };
                double computedResult = integrate(f, *intType);
                BOOST_CHECK_CLOSE(computedResult, ExactIntegralMonomialTetrahedron(n, count), 1.e-9);
                count++;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(IntegrateTetrahedron)
{
    int maxOrder = 18;
    for (int i = 1; i <= maxOrder; i++)
    {
        CheckTetrahedronIntegration(i, i);
        std::cout << "Done: order " << i << std::endl;
    }
}
