//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHeatConduction.h"

registerADMooseObject("HeatConductionApp", ADHeatConduction);

defineADLegacyParams(ADHeatConduction);

template <ComputeStage compute_stage>
InputParameters
ADHeatConduction<compute_stage>::validParams()
{
  InputParameters params = ADDiffusion<compute_stage>::validParams();
  params.addParam<MaterialPropertyName>("thermal_conductivity",
                                        "thermal_conductivity",
                                        "the name of the thermal conductivity material property");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

template <ComputeStage compute_stage>
ADHeatConduction<compute_stage>::ADHeatConduction(const InputParameters & parameters)
  : ADDiffusion<compute_stage>(parameters),
    _thermal_conductivity(getADMaterialProperty<Real>("thermal_conductivity"))
{
}

template <ComputeStage compute_stage>
ADRealVectorValue
ADHeatConduction<compute_stage>::precomputeQpResidual()
{
  return _thermal_conductivity[_qp] * ADDiffusion<compute_stage>::precomputeQpResidual();
}
