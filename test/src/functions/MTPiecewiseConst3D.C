//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MTPiecewiseConst3D.h"

registerMooseObject("MooseTestApp", MTPiecewiseConst3D);

InputParameters
MTPiecewiseConst3D::validParams()
{
  InputParameters params = Function::validParams();
  return params;
}

MTPiecewiseConst3D::MTPiecewiseConst3D(const InputParameters & parameters) : Function(parameters) {}

Real
MTPiecewiseConst3D::value(Real /*t*/, const Point & p) const
{
  Real val = 0;
  Real x = p(0);
  Real y = p(1);
  Real z = p(2);

  Real one_third = 1. / 3.;

  if ((x >= -0.75 && x < -0.50) || (x >= -0.25 && x < 0.25) || (x >= 0.50 && x < 0.75))
    val += one_third;

  if ((y >= -0.75 && y < -0.50) || (y >= -0.25 && y < 0.25) || (y >= 0.50 && y < 0.75))
    val += one_third;

  if ((z >= -0.75 && z < -0.50) || (z >= -0.25 && z < 0.25) || (z >= 0.50 && z < 0.75))
    val += one_third;

  return val;
}
