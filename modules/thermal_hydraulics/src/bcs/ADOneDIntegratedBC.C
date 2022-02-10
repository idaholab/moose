//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADOneDIntegratedBC.h"

InputParameters
ADOneDIntegratedBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<Real>("normal", "Component of outward normal along 1-D direction");
  return params;
}

ADOneDIntegratedBC::ADOneDIntegratedBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _normal(getParam<Real>("normal"))
{
}
