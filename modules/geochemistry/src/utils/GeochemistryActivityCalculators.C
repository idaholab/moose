//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryActivityCalculators.h"
#include "libmesh/utility.h"
#include "GeochemistryConstants.h"

namespace GeochemistryActivityCalculators
{
Real
log10ActCoeffDHBdot(Real charge, Real ion_size, Real sqrt_ionic_strength, Real A, Real B, Real Bdot)
{
  if (sqrt_ionic_strength <= 0.0)
    return 0.0; // guard against unphysical inputs that might occur during a Newton process
  return -A * Utility::pow<2>(charge) * sqrt_ionic_strength /
             (1.0 + ion_size * B * sqrt_ionic_strength) +
         Bdot * Utility::pow<2>(sqrt_ionic_strength);
}

Real
log10ActCoeffDHBdotNeutral(Real ionic_strength, Real a, Real b, Real c, Real d)
{
  if (ionic_strength <= 0.0)
    return 0.0; // guard against unphysical inputs that might occur during a Newton process
  return a * ionic_strength + b * Utility::pow<2>(ionic_strength) +
         c * Utility::pow<3>(ionic_strength) + d * Utility::pow<4>(ionic_strength);
}

Real
log10ActCoeffDHBdotAlternative(Real ionic_strength, Real Bdot)
{
  if (ionic_strength <= 0.0)
    return 0.0; // guard against unphysical inputs that might occur during a Newton process
  return Bdot * ionic_strength;
}

Real
log10ActCoeffDavies(Real charge, Real sqrt_ionic_strength, Real A)
{
  if (sqrt_ionic_strength <= 0.0)
    return 0.0; // guard against unphysical inputs that might occur during a Newton process
  return -A * Utility::pow<2>(charge) *
         (sqrt_ionic_strength / (1.0 + sqrt_ionic_strength) -
          0.3 * Utility::pow<2>(sqrt_ionic_strength));
}

Real
lnActivityDHBdotWater(
    Real stoichiometric_ionic_strength, Real A, Real atilde, Real btilde, Real ctilde, Real dtilde)
{
  if (stoichiometric_ionic_strength <= 0.0)
    return 0.0; // guard against unphysical inputs that might occur during a Newton process
  const Real bhat = 1.0 + atilde * std::sqrt(stoichiometric_ionic_strength);
  const Real inner = bhat - 2.0 * std::log(bhat) - 1.0 / bhat;
  const Real outer = 1.0 -
                     A * GeochemistryConstants::LOGTEN / Utility::pow<3>(atilde) /
                         stoichiometric_ionic_strength * inner +
                     0.5 * btilde * stoichiometric_ionic_strength +
                     2.0 * ctilde * Utility::pow<2>(stoichiometric_ionic_strength) / 3.0 +
                     0.75 * dtilde * Utility::pow<3>(stoichiometric_ionic_strength);
  return -2.0 * stoichiometric_ionic_strength * outer / GeochemistryConstants::MOLES_PER_KG_WATER;
}

} // namespace GeochemistryActivityCalculators
