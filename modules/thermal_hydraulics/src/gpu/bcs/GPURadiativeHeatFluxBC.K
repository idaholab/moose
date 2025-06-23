//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPURadiativeHeatFluxBC.h"

registerMooseObject("HeatTransferApp", GPURadiativeHeatFluxBC);

InputParameters
GPURadiativeHeatFluxBC::validParams()
{
  InputParameters params = GPURadiativeHeatFluxBCBase::validParams();

  params.addParam<PostprocessorName>(
      "scale_pp", 1.0, "Post-processor by which to scale boundary condition");

  params.addClassDescription(
      "Radiative heat transfer boundary condition for a plate heat structure");

  return params;
}

GPURadiativeHeatFluxBC::GPURadiativeHeatFluxBC(const InputParameters & parameters)
  : GPURadiativeHeatFluxBCBase(parameters), _scale_pp(getPostprocessorValue("scale_pp"))
{
}
