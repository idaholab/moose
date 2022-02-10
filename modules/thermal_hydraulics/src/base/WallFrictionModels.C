//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WallFrictionModels.h"
#include "Numerics.h"
#include "ADReal.h"

namespace WallFriction
{

Real
DarcyFrictionFactor(const Real & f_F)
{
  return 4 * f_F;
}

ADReal
DarcyFrictionFactor(const ADReal & f_F)
{
  return 4 * f_F;
}

Real
FanningFrictionFactorChurchill(Real Re, Real roughness, Real Dh)
{
  Real Re_limit = std::max(Re, 10.0);

  Real a =
      std::pow(2.457 * std::log(1.0 / (std::pow(7.0 / Re_limit, 0.9) + 0.27 * roughness / Dh)), 16);
  Real b = std::pow(3.753e4 / Re_limit, 16);
  return 2.0 * std::pow(std::pow(8.0 / Re_limit, 12) + 1.0 / std::pow(a + b, 1.5), 1.0 / 12.0);
}

ADReal
FanningFrictionFactorChurchill(ADReal Re, ADReal roughness, ADReal Dh)
{
  ADReal Re_limit = std::max(Re, 10.0);

  ADReal a =
      std::pow(2.457 * std::log(1.0 / (std::pow(7.0 / Re_limit, 0.9) + 0.27 * roughness / Dh)), 16);
  ADReal b = std::pow(3.753e4 / Re_limit, 16);
  return 2.0 * std::pow(std::pow(8.0 / Re_limit, 12) + 1.0 / std::pow(a + b, 1.5), 1.0 / 12.0);
}
}
