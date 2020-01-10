//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeDilatationThermalExpansionEigenstrainBase.h"

defineADLegacyParams(ADComputeDilatationThermalExpansionEigenstrainBase);

template <ComputeStage compute_stage>
InputParameters
ADComputeDilatationThermalExpansionEigenstrainBase<compute_stage>::validParams()
{
  return ADComputeThermalExpansionEigenstrainBase<compute_stage>::validParams();
}

template <ComputeStage compute_stage>
ADComputeDilatationThermalExpansionEigenstrainBase<compute_stage>::
    ADComputeDilatationThermalExpansionEigenstrainBase(const InputParameters & parameters)
  : ADComputeThermalExpansionEigenstrainBase<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
void
ADComputeDilatationThermalExpansionEigenstrainBase<compute_stage>::computeThermalStrain(
    ADReal & thermal_strain)
{
  thermal_strain =
      computeDilatation(_temperature[_qp]) - computeDilatation(_stress_free_temperature[_qp]);
}

adBaseClass(ADComputeDilatationThermalExpansionEigenstrainBase);
