//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothTransition.h"

SmoothTransition::SmoothTransition(const Real & x_center, const Real & transition_width)
  : _x_center(x_center),
    _transition_width(transition_width),
    _x1(_x_center - 0.5 * _transition_width),
    _x2(_x_center + 0.5 * _transition_width)
{
}
