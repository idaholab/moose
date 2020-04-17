//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRobinBC.h"

registerMooseObject("MooseTestApp", ADRobinBC);

InputParameters
ADRobinBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  return params;
}

ADRobinBC::ADRobinBC(const InputParameters & parameters) : ADIntegratedBC(parameters) {}

ADReal
ADRobinBC::computeQpResidual()
{
  return _test[_i][_qp] * 2. * _u[_qp];
}
