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

defineADValidParams(
    ADHeatConduction,
    ADDiffusion,
    params.addParam<MaterialPropertyName>("thermal_conductivity",
                                          "thermal_conductivity",
                                          "the name of the thermal conductivity material property");
    params.addParam<RealVectorValue>("axis_scaling_vector", "todo");
    params.set<bool>("use_displaced_mesh") = true;);

template <ComputeStage compute_stage>
ADHeatConduction<compute_stage>::ADHeatConduction(const InputParameters & parameters)
  : ADDiffusion<compute_stage>(parameters),
    _thermal_conductivity(getADMaterialProperty<Real>("thermal_conductivity"))
{
  _scaling_vector = {1.0, 1.0, 1.0};
  if (isParamValid("axis_scaling_vector"))
  {
    _scaling_vector = getParam<RealVectorValue>("axis_scaling_vector");

    _scaling_vector(0) *= _scaling_vector(0);
    _scaling_vector(1) *= _scaling_vector(1);
    _scaling_vector(2) *= _scaling_vector(2);
  }
}

template <ComputeStage compute_stage>
ADRealVectorValue
ADHeatConduction<compute_stage>::precomputeQpResidual()
{
  ADRealVectorValue v = ADDiffusion<compute_stage>::precomputeQpResidual();
  v(0) *= _scaling_vector(0);
  v(1) *= _scaling_vector(1);
  v(2) *= _scaling_vector(2);

  return _thermal_conductivity[_qp] * v;
}
