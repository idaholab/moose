//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WeightedTransition.h"

WeightedTransition::WeightedTransition(const Real & x_center, const Real & transition_width)
  : SmoothTransition(x_center, transition_width)
{
}

Real
WeightedTransition::value(const Real & x, const Real & f1, const Real & f2) const
{
  if (x <= _x1)
    return f1;
  else if (x >= _x2)
    return f2;
  else
  {
    const Real w = weight(x);
    return w * f1 + (1.0 - w) * f2;
  }
}

Real
WeightedTransition::derivative(
    const Real & x, const Real & f1, const Real & f2, const Real & df1dx, const Real & df2dx) const
{
  if (x <= _x1)
    return df1dx;
  else if (x >= _x2)
    return df2dx;
  else
  {
    const Real w = weight(x);
    const Real dwdx = -0.5 * std::sin(M_PI / (_x2 - _x1) * (x - _x1)) * M_PI / (_x2 - _x1);
    return w * df1dx + (1.0 - w) * df2dx + dwdx * (f1 - f2);
  }
}

Real
WeightedTransition::weight(const Real & x) const
{
  return 0.5 * (std::cos(M_PI / (_x2 - _x1) * (x - _x1)) + 1.0);
}
