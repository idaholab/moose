//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FixedPointSteady.h"

registerMooseObject("MooseTestApp", FixedPointSteady);

template <>
InputParameters
validParams<FixedPointSteady>()
{
  InputParameters params = validParams<Steady>();
  params += validParams<FixedPoint>();
  return params;
}

FixedPointSteady::FixedPointSteady(const InputParameters & parameters)
  : Steady(parameters), _fixed_point(this)
{
  _fixed_point.setInnerSolve(_feproblem_solve);
  _picard_solve.setInnerSolve(_fixed_point);
}
