//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVectorRobinBC.h"

registerMooseObject("MooseTestApp", ADVectorRobinBC);

InputParameters
ADVectorRobinBC::validParams()
{
  InputParameters params = ADVectorIntegratedBC::validParams();
  return params;
}

ADVectorRobinBC::ADVectorRobinBC(const InputParameters & parameters)
  : ADVectorIntegratedBC(parameters)
{
}

ADReal
ADVectorRobinBC::computeQpResidual()
{
  return _test[_i][_qp] * 2. * _u[_qp];
}
