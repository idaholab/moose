//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceDiffusionBase.h"

InputParameters
InterfaceDiffusionBase::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addParam<Real>("D", 1.0, "Diffusion coefficient");
  params.addParam<Real>("D_neighbor", 1.0, "Neighbor variable diffusion coefficient");
  return params;
}

InterfaceDiffusionBase::InterfaceDiffusionBase(const InputParameters & parameters)
  : InterfaceKernel(parameters), _D(getParam<Real>("D")), _D_neighbor(getParam<Real>("D_neighbor"))
{
}
