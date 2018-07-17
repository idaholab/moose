//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowBrooksCorey.h"

namespace PorousFlowBrooksCorey
{
Real
effectiveSaturation(Real pc, Real pe, Real lambda)
{
  if (pc < pe)
    return 1.0;
  else
    return std::pow(pc / pe, -lambda);
}

Real
dEffectiveSaturation(Real pc, Real pe, Real lambda)
{
  if (pc < pe)
    return 0.0;
  else
    return -lambda * std::pow(pc / pe, -lambda - 1.0) / pe;
}

Real
d2EffectiveSaturation(Real pc, Real pe, Real lambda)
{
  if (pc < pe)
    return 0.0;
  else
    return lambda * (lambda + 1.0) * std::pow(pc / pe, -lambda - 2.0) / pe / pe;
}

Real
capillaryPressure(Real seff, Real pe, Real lambda, Real pc_max)
{
  if (seff >= 1.0)
    return 0.0;
  else if (seff <= 0.0)
    return pc_max;
  else
    return std::min(pe * std::pow(seff, -1.0 / lambda), pc_max);
}

Real
dCapillaryPressure(Real seff, Real pe, Real lambda, Real pc_max)
{
  if (seff <= 0.0 || seff >= 1.0)
    return 0.0;
  else
  {
    // Return 0 if pc > pc_max
    if (capillaryPressure(seff, pe, lambda, pc_max) >= pc_max)
      return 0.0;
    else
      return -pe * std::pow(seff, -1.0 / lambda - 1.0) / lambda;
  }
}

Real
d2CapillaryPressure(Real seff, Real pe, Real lambda, Real pc_max)
{
  if (seff <= 0.0 || seff >= 1.0)
    return 0.0;
  else
  {
    // Return 0 if pc > pc_max
    if (capillaryPressure(seff, pe, lambda, pc_max) >= pc_max)
      return 0.0;
    else
      return (lambda + 1.0) * pe * std::pow(seff, -1.0 / lambda - 2.0) / lambda / lambda;
  }
}

Real
relativePermeabilityW(Real seff, Real lambda)
{
  if (seff <= 0.0)
    return 0.0;
  else if (seff >= 1.0)
    return 1.0;

  return std::pow(seff, (2.0 + 3.0 * lambda) / lambda);
}

Real
dRelativePermeabilityW(Real seff, Real lambda)
{
  // Guard against division by zero
  if (seff <= 0.0 || seff >= 1.0)
    return 0.0;

  return (2.0 + 3.0 * lambda) * std::pow(seff, (2.0 + 2.0 * lambda) / lambda) / lambda;
}

Real
relativePermeabilityNW(Real seff, Real lambda)
{
  if (seff <= 0.0)
    return 0.0;
  else if (seff >= 1.0)
    return 1.0;

  return seff * seff * (1.0 - std::pow(1.0 - seff, (2.0 + lambda) / lambda));
}

Real
dRelativePermeabilityNW(Real seff, Real lambda)
{
  // Guard against division by zero
  if (seff <= 0.0 || seff >= 1.0)
    return 0.0;

  return seff * (2.0 + (seff * (2.0 + 3.0 * lambda) - 2.0 * lambda) *
                           std::pow(1.0 - seff, 2.0 / lambda) / lambda);
}
} // namespace PorousFlowBrooksCorey
