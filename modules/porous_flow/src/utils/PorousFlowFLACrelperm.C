/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFLACrelperm.h"

namespace PorousFlowFLACrelperm
{
Real
relativePermeability(Real seff, Real m)
{
  if (seff <= 0.0)
    return 0.0;
  else if (seff >= 1.0)
    return 1.0;
  return (1.0 + m) * std::pow(seff, m) - m * std::pow(seff, m + 1.0);
}

Real
dRelativePermeability(Real seff, Real m)
{
  if (seff <= 0.0 || seff >= 1.0)
    return 0.0;
  return (1.0 + m) * m * std::pow(seff, m - 1.0) - m * (m + 1.0) * std::pow(seff, m);
}

Real
d2RelativePermeability(Real seff, Real m)
{
  if (seff <= 0.0 || seff >= 1.0)
    return 0.0;
  return (1.0 + m) * m * (m - 1.0) * std::pow(seff, m - 2.0) -
         m * (m + 1.0) * m * std::pow(seff, m - 1.0);
}
}
