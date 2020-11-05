//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVNeumannBC.h"

registerMooseObject("MooseApp", FVNeumannBC);

InputParameters
FVNeumannBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Neumann boundary condition for finite volume method.");
  params.addParam<Real>("value", 0.0, "The value of the flux crossing the boundary.");
  return params;
}

FVNeumannBC::FVNeumannBC(const InputParameters & parameters)
  : FVFluxBC(parameters), _value(getParam<Real>("value"))
{
}

ADReal
FVNeumannBC::computeQpResidual()
{
  return -_value;
}
