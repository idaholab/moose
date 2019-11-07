//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BiharmonicLapBC.h"
#include "Function.h"

registerMooseObject("MooseTestApp", BiharmonicLapBC);

InputParameters
BiharmonicLapBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addParam<FunctionName>(
      "laplacian_function", "0", "A function representing the weakly-imposed Laplacian.");
  return params;
}

BiharmonicLapBC::BiharmonicLapBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _lap_u(getFunction("laplacian_function"))
{
}

Real
BiharmonicLapBC::computeQpResidual()
{
  return -_lap_u.value(_t, _q_point[_qp]) * (_grad_test[_i][_qp] * _normals[_qp]);
}
