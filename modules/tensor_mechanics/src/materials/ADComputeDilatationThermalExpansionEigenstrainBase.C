//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeDilatationThermalExpansionEigenstrainBase.h"

InputParameters
ADComputeDilatationThermalExpansionEigenstrainBase::validParams()
{
  auto params = ADComputeThermalExpansionEigenstrainBase::validParams();
  return params;
}

ADComputeDilatationThermalExpansionEigenstrainBase::
    ADComputeDilatationThermalExpansionEigenstrainBase(const InputParameters & parameters)
  : ADComputeThermalExpansionEigenstrainBase(parameters)
{
}

void
ADComputeDilatationThermalExpansionEigenstrainBase::computeThermalStrain(ADReal & thermal_strain,
                                                                         Real *)
{
  if (_use_old_temperature)
    thermal_strain =
        computeDilatation(_temperature_old[_qp]) - computeDilatation(_stress_free_temperature[_qp]);
  else
    thermal_strain =
        computeDilatation(_temperature[_qp]) - computeDilatation(_stress_free_temperature[_qp]);
}
