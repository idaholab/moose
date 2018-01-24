//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowRogersStallybrassClements.h"

namespace PorousFlowRogersStallybrassClements
{
Real
effectiveSaturation(Real pc, Real shift, Real scale)
{
  Real x = (pc - shift) / scale;
  Real ex = std::exp(x);
  return std::pow(1.0 + ex, -0.5);
}

Real
dEffectiveSaturation(Real pc, Real shift, Real scale)
{
  Real x = (pc - shift) / scale;
  Real ex = std::exp(x);
  return -0.5 * ex * std::pow(1.0 + ex, -1.5) / scale;
}

Real
d2EffectiveSaturation(Real pc, Real shift, Real scale)
{
  Real x = (pc - shift) / scale;
  Real ex = std::exp(x);
  return (0.75 * ex * ex * std::pow(1.0 + ex, -2.5) - 0.5 * ex * std::pow(1.0 + ex, -1.5)) / scale /
         scale;
}
}
