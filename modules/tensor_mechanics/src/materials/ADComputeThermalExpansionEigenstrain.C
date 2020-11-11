//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeThermalExpansionEigenstrain.h"

registerMooseObject("TensorMechanicsApp", ADComputeThermalExpansionEigenstrain);

InputParameters
ADComputeThermalExpansionEigenstrain::validParams()
{
  InputParameters params = ADComputeThermalExpansionEigenstrainBase::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion "
                             "with a constant coefficient");
  params.addRequiredParam<Real>("thermal_expansion_coeff", "Thermal expansion coefficient");
  return params;
}

ADComputeThermalExpansionEigenstrain::ADComputeThermalExpansionEigenstrain(
    const InputParameters & parameters)
  : ADComputeThermalExpansionEigenstrainBase(parameters),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff"))
{
}

void
ADComputeThermalExpansionEigenstrain::computeThermalStrain(ADReal & thermal_strain)
{
  if (_use_old_temperature)
    thermal_strain =
        _thermal_expansion_coeff * (_temperature_old[_qp] - _stress_free_temperature[_qp]);
  else
    thermal_strain = _thermal_expansion_coeff * (_temperature[_qp] - _stress_free_temperature[_qp]);
}
