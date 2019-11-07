//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MTPiecewiseConst2D.h"

registerMooseObject("MooseTestApp", MTPiecewiseConst2D);

InputParameters
MTPiecewiseConst2D::validParams()
{
  InputParameters params = Function::validParams();
  return params;
}

MTPiecewiseConst2D::MTPiecewiseConst2D(const InputParameters & parameters) : Function(parameters) {}

Real
MTPiecewiseConst2D::value(Real /*t*/, const Point & p) const
{
  Real val = 0;
  Real x = p(0);
  Real y = p(1);

  if ((x >= -0.6 && x < 0.2 && y >= 0.65 && y < 0.75) ||
      (x >= -0.6 && x < 0.2 && y >= 0.0 && y < 0.1) ||
      (x >= -0.6 && x < -.5 && y >= 0.0 && y < 0.75) ||
      (x >= 0.1 && x < 0.2 && y >= 0.0 && y < 0.75))
    val += 0.3;

  if ((x >= -0.1 && x < 0.65 && y >= 0.25 && y < 0.35) ||
      (x >= -0.1 && x < 0.65 && y >= -.15 && y < -.05) ||
      (x >= -0.1 && x < 0.65 && y >= -.55 && y < -.45) ||
      (x >= -0.1 && x < 0.0 && y >= -.15 && y < 0.35) ||
      (x >= 0.55 && x < 0.65 && y >= -.55 && y < -.05))
    val += 0.5;

  return val;
}
