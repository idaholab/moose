//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.htmlo
#include "ADHeatConduction.h"

registerADMooseObject("HeatConductionApp", ADHeatConduction);

defineADValidParams(ADHeatConduction,
                    ADDiffusion,
                    params.addRequiredParam<MaterialPropertyName>(
                        "thermal_conductivity",
                        "the name of the thermal conductivity material property"););

template <ComputeStage compute_stage>
ADHeatConduction<compute_stage>::ADHeatConduction(const InputParameters & parameters)
  : ADDiffusion<compute_stage>(parameters),
    _thermal_conductivity(adGetADMaterialProperty<Real>("thermal_conductivity"))
{
}

template <ComputeStage compute_stage>
ADResidual
ADHeatConduction<compute_stage>::computeQpResidual()
{
  return _thermal_conductivity[_qp] * ADDiffusion<compute_stage>::computeQpResidual();
}
