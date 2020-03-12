//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectiveHeatFluxBC.h"

registerADMooseObject("HeatConductionApp", ADConvectiveHeatFluxBC);

defineADLegacyParams(ADConvectiveHeatFluxBC);

template <ComputeStage compute_stage>
InputParameters
ADConvectiveHeatFluxBC<compute_stage>::validParams()
{
  InputParameters params = ADIntegratedBC<compute_stage>::validParams();
  params.addClassDescription(
      "Convective heat transfer boundary condition with temperature and heat "
      "transfer coefficent given by material properties.");
  params.addRequiredParam<MaterialPropertyName>("T_infinity",
                                                "Material property for far-field temperature");
  params.addRequiredParam<MaterialPropertyName>("heat_transfer_coefficient",
                                                "Material property for heat transfer coefficient");
  params.addParam<MaterialPropertyName>(
      "heat_transfer_coefficient_dT",
      "0",
      "Material property for derivative of heat transfer coefficient with respect to temperature");

  return params;
}

template <ComputeStage compute_stage>
ADConvectiveHeatFluxBC<compute_stage>::ADConvectiveHeatFluxBC(const InputParameters & parameters)
  : ADIntegratedBC<compute_stage>(parameters),
    _T_infinity(getADMaterialProperty<Real>("T_infinity")),
    _htc(getADMaterialProperty<Real>("heat_transfer_coefficient")),
    _htc_dT(getADMaterialProperty<Real>("heat_transfer_coefficient_dT"))
{
}

template <ComputeStage compute_stage>
ADReal
ADConvectiveHeatFluxBC<compute_stage>::computeQpResidual()
{
  return -_test[_i][_qp] * _htc[_qp] * (_T_infinity[_qp] - _u[_qp]);
}
