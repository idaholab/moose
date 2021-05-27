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

InputParameters
FixedPointSteady::validParams()
{
  InputParameters params = Steady::validParams();
  params += FixedPoint::validParams();
  return params;
}

FixedPointSteady::FixedPointSteady(const InputParameters & parameters)
  : Steady(parameters), _fixed_point(*this)
{
  _fixed_point.setInnerSolve(_feproblem_solve);
  _fixed_point_solve->setInnerSolve(_fixed_point);
}
