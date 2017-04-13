/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
