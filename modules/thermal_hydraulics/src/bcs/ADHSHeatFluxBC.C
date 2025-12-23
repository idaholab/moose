//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHSHeatFluxBC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADHSHeatFluxBC);

InputParameters
ADHSHeatFluxBC::validParams()
{
  InputParameters params = ADFunctionNeumannBC::validParams();

  params.addParam<FunctionName>("scale", 1.0, "Function by which to scale the boundary condition");

  params.addClassDescription("Applies a specified heat flux to the side of a plate heat structure");

  return params;
}

ADHSHeatFluxBC::ADHSHeatFluxBC(const InputParameters & parameters)
  : ADFunctionNeumannBC(parameters), _scale_fn(getFunction("scale"))
{
}

ADReal
ADHSHeatFluxBC::computeQpResidual()
{
  return _scale_fn.value(_t, _q_point[_qp]) * ADFunctionNeumannBC::computeQpResidual();
}
