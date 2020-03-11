//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCHSplitConcentration.h"

registerADMooseObject("PhaseFieldApp", ADCHSplitConcentration);

defineADLegacyParams(ADCHSplitConcentration);

template <ComputeStage compute_stage>
InputParameters
ADCHSplitConcentration<compute_stage>::validParams()
{
  InputParameters params = ADKernel<compute_stage>::validParams();
  params.addClassDescription("Concentration kernel in Split Cahn-Hilliard that solves chemical "
                             "potential in a weak form");
  params.addRequiredCoupledVar("chemical_potential_var", "Chemical potential variable");
  params.addRequiredParam<MaterialPropertyName>("mobility", "Mobility property name");
  return params;
}

template <ComputeStage compute_stage>
ADCHSplitConcentration<compute_stage>::ADCHSplitConcentration(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters),
    _grad_mu(adCoupledGradient("chemical_potential_var")),
    _mobility(getADMaterialProperty<Real>("mobility"))
{
}

template <ComputeStage compute_stage>
ADReal
ADCHSplitConcentration<compute_stage>::computeQpResidual()
{
  return _mobility[_qp] * _grad_mu[_qp] * _grad_test[_i][_qp];
}
