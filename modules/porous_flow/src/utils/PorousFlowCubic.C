//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowCubic.h"
#include "libmesh/utility.h"

namespace PorousFlowCubic
{
Real
cubic(Real x, Real x0, Real y0, Real y0p, Real x1, Real y1, Real y1p)
{
  mooseAssert(x0 != x1, "PorousFlowCubic: x0 cannot equal x1");
  const Real d = x1 - x0;
  const Real d2 = Utility::pow<2>(d);
  const Real mean = 0.5 * (x1 + x0);
  const Real sq3 = 0.5 * std::sqrt(3.0) * d;
  const Real term1 = y0p * (x - x0) * Utility::pow<2>(x - x1) /
                     d2; // term1(x0) = term1(x1) = term1'(x1) = 0, term1'(x0) = y0p
  const Real term2 = y1p * (x - x1) * Utility::pow<2>(x - x0) /
                     d2; // term2(x0) = term2(x1) = term2'(x0) = 0, term2'(x1) = y1p
  const Real term3 = (x - mean - sq3) * (x - mean) * (x - mean + sq3);
  // term3' = (x - mean) * (x - mean + sq3) + (x - mean - sq3) * (x - mean + sq3) + (x - mean - sq3)
  // * (x - mean)
  //        = 3 (x - mean)^2 - sq3^2
  // note term3' = 0 when x = mean +/- sq3/sqrt(3) = 0.5 * (x1 + x0) +/- 0.5 * (x1 - x0) = {x1, x0}
  const Real term3_x0 = (x0 - mean - sq3) * (x0 - mean) * (x0 - mean + sq3);
  const Real term3_x1 = (x1 - mean - sq3) * (x1 - mean) * (x1 - mean + sq3);
  return (y0 * (term3 - term3_x1) + y1 * (term3_x0 - term3)) / (term3_x0 - term3_x1) + term1 +
         term2;
}

Real
dcubic(Real x, Real x0, Real y0, Real y0p, Real x1, Real y1, Real y1p)
{
  mooseAssert(x0 != x1, "PorousFlowCubic: x0 cannot equal x1");
  const Real d = x1 - x0;
  const Real d2 = Utility::pow<2>(d);
  const Real mean = 0.5 * (x1 + x0);
  const Real sq3 = 0.5 * std::sqrt(3.0) * d;
  const Real term1_prime = y0p * (Utility::pow<2>(x - x1) + (x - x0) * 2 * (x - x1)) / d2;
  const Real term2_prime = y1p * (Utility::pow<2>(x - x0) + (x - x1) * 2 * (x - x0)) / d2;
  const Real term3_prime =
      3.0 * Utility::pow<2>(mean) - 6 * mean * x - 0.75 * d2 + 3.0 * Utility::pow<2>(x);
  const Real term3_x0 = (x0 - mean - sq3) * (x0 - mean) * (x0 - mean + sq3);
  const Real term3_x1 = (x1 - mean - sq3) * (x1 - mean) * (x1 - mean + sq3);
  return (y0 - y1) * term3_prime / (term3_x0 - term3_x1) + term1_prime + term2_prime;
}
}
