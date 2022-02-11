//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWeightedTransition.h"

ADWeightedTransition::ADWeightedTransition(const ADReal & x_center, const ADReal & transition_width)
  : ADSmoothTransition(x_center, transition_width)
{
}

ADReal
ADWeightedTransition::value(const ADReal & x, const ADReal & f1, const ADReal & f2) const
{
  if (x <= _x1)
    return f1;
  else if (x >= _x2)
    return f2;
  else
  {
    const ADReal w = weight(x);
    return w * f1 + (1.0 - w) * f2;
  }
}

ADReal
ADWeightedTransition::weight(const ADReal & x) const
{
  return 0.5 * (std::cos(libMesh::pi / (_x2 - _x1) * (x - _x1)) + 1.0);
}
