//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UnaryRecombinationBC.h"

registerMooseObject("ScalarTransportApp", UnaryRecombinationBC);

InputParameters
UnaryRecombinationBC::validParams()
{
  auto params = ADIntegratedBC::validParams();
  params.addParam<Real>("Kr", 1, "The recombination coefficient");
  return params;
}

UnaryRecombinationBC::UnaryRecombinationBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _Kr(getParam<Real>("Kr"))
{
}

ADReal
UnaryRecombinationBC::computeQpResidual()
{
  return _test[_i][_qp] * _Kr * _u[_qp] * _u[_qp];
}
