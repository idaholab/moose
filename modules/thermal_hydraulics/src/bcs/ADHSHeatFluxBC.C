//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHSHeatFluxBC.h"

registerMooseObject("ThermalHydraulicsApp", ADHSHeatFluxBC);

InputParameters
ADHSHeatFluxBC::validParams()
{
  InputParameters params = ADFunctionNeumannBC::validParams();

  params.addParam<PostprocessorName>(
      "scale_pp", 1.0, "Post-processor by which to scale boundary condition");

  params.addClassDescription("Applies a specified heat flux to the side of a plate heat structure");

  return params;
}

ADHSHeatFluxBC::ADHSHeatFluxBC(const InputParameters & parameters)
  : ADFunctionNeumannBC(parameters),

    _scale_pp(getPostprocessorValue("scale_pp"))
{
}

ADReal
ADHSHeatFluxBC::computeQpResidual()
{
  return _scale_pp * ADFunctionNeumannBC::computeQpResidual();
}
