//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BinaryRecombinationBC.h"

registerMooseObject("ScalarTransportApp", BinaryRecombinationBC);

InputParameters
BinaryRecombinationBC::validParams()
{
  auto params = ADIntegratedBC::validParams();
  params.addClassDescription("Models recombination of the variable with a coupled species at "
                             "boundaries, resulting in loss");
  params.addRequiredCoupledVar("v", "The other mobile variable that takes part in recombination");
  params.addParam<Real>("Kr", 1, "The recombination coefficient");
  return params;
}

BinaryRecombinationBC::BinaryRecombinationBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _v(adCoupledValue("v")), _Kr(getParam<Real>("Kr"))
{
}

ADReal
BinaryRecombinationBC::computeQpResidual()
{
  return _test[_i][_qp] * _Kr * _u[_qp] * _v[_qp];
}
