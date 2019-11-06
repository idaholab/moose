//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TEIC.h"

registerMooseObject("MooseTestApp", TEIC);

InputParameters
TEIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addParam<double>("t_jump", 1.0, "Time when the jump occurs");
  params.addParam<double>("slope", 1.0, "How steep the jump is");
  return params;
}

TEIC::TEIC(const InputParameters & parameters)
  : InitialCondition(parameters), _t_jump(getParam<Real>("t_jump")), _slope(getParam<Real>("slope"))
{
}

Real
TEIC::value(const Point & /*p*/)
{
  Real t = 0.0;
  return atan((t - _t_jump) * libMesh::pi * _slope);
}
