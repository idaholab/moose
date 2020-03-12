//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCHSoretMobility.h"

registerADMooseObject("PhaseFieldApp", ADCHSoretMobility);

defineADLegacyParams(ADCHSoretMobility);

template <ComputeStage compute_stage>
InputParameters
ADCHSoretMobility<compute_stage>::validParams()
{
  InputParameters params = ADKernel<compute_stage>::validParams();
  params.addClassDescription("Adds contribution due to thermo-migration to the Cahn-Hilliard "
                             "equation using a concentration 'u', temperature 'T', and thermal "
                             "mobility 'mobility' (in units of length squared per time).");
  params.addRequiredCoupledVar("T", "The temperature variable");
  params.addRequiredParam<MaterialPropertyName>("mobility", "The mobility property name");
  return params;
}

template <ComputeStage compute_stage>
ADCHSoretMobility<compute_stage>::ADCHSoretMobility(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters),
    _T(adCoupledValue("T")),
    _grad_T(adCoupledGradient("T")),
    _mobility(getADMaterialProperty<Real>("mobility"))
{
}

template <ComputeStage compute_stage>
ADReal
ADCHSoretMobility<compute_stage>::computeQpResidual()
{
  return _mobility[_qp] * _grad_T[_qp] / _T[_qp] * _grad_test[_i][_qp];
}
