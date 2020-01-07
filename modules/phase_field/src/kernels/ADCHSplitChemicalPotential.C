//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCHSplitChemicalPotential.h"

registerADMooseObject("PhaseFieldApp", ADCHSplitChemicalPotential);

defineADValidParams(
    ADCHSplitChemicalPotential,
    ADKernel,
    params.addClassDescription("Chemical potential kernel in Split Cahn-Hilliard that solves "
                               "chemical potential in a weak form");
    params.addRequiredParam<MaterialPropertyName>("chemical_potential",
                                                  "Chemical potential property name"););

template <ComputeStage compute_stage>
ADCHSplitChemicalPotential<compute_stage>::ADCHSplitChemicalPotential(
    const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters),
    _chemical_potential(getADMaterialProperty<Real>("chemical_potential"))
{
}

template <ComputeStage compute_stage>
ADReal
ADCHSplitChemicalPotential<compute_stage>::computeQpResidual()
{
  return _test[_i][_qp] * (_u[_qp] - _chemical_potential[_qp]);
}
