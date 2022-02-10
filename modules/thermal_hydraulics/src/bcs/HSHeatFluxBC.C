//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSHeatFluxBC.h"

registerMooseObject("ThermalHydraulicsApp", HSHeatFluxBC);

InputParameters
HSHeatFluxBC::validParams()
{
  InputParameters params = FunctionNeumannBC::validParams();

  params.addParam<PostprocessorName>(
      "scale_pp", 1.0, "Post-processor by which to scale boundary condition");

  params.addClassDescription("Applies a specified heat flux to the side of a plate heat structure");

  return params;
}

HSHeatFluxBC::HSHeatFluxBC(const InputParameters & parameters)
  : FunctionNeumannBC(parameters),

    _scale_pp(getPostprocessorValue("scale_pp"))
{
}

Real
HSHeatFluxBC::computeQpResidual()
{
  return _scale_pp * FunctionNeumannBC::computeQpResidual();
}
