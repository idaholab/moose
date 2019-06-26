//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionOutflow.h"

registerADMooseObject("DarcyThermoMechApp", HeatConductionOutflow);

defineADValidParams(HeatConductionOutflow,
                    ADIntegratedBC,
                    params.addClassDescription("Compute the outflow boundary condition."););

template <ComputeStage compute_stage>
HeatConductionOutflow<compute_stage>::HeatConductionOutflow(const InputParameters & parameters)
  : ADIntegratedBC<compute_stage>(parameters),
    _thermal_conductivity(getADMaterialProperty<Real>("thermal_conductivity"))
{
}

template <ComputeStage compute_stage>
ADReal
HeatConductionOutflow<compute_stage>::computeQpResidual()
{
  return -_test[_i][_qp] * _thermal_conductivity[_qp] * _grad_u[_qp] * _normals[_qp];
}
