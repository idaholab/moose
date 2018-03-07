//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeBeamThermalExpansionEigenstrain.h"

template <>
InputParameters
validParams<ComputeBeamThermalExpansionEigenstrain>()
{
  InputParameters params = validParams<ComputeBeamThermalExpansionEigenstrainBase>();
  params.addClassDescription("Computes eigenstrain due to thermal expansion "
                             "with a constant coefficient");
  params.addParam<Real>("thermal_expansion_coeff", "Thermal expansion coefficient");

  return params;
}

ComputeBeamThermalExpansionEigenstrain::ComputeBeamThermalExpansionEigenstrain(
    const InputParameters & parameters)
  : ComputeBeamThermalExpansionEigenstrainBase(parameters),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff"))
{
}

void
ComputeBeamThermalExpansionEigenstrain::computeThermalStrain(Real & thermal_strain,
                                                             Real & instantaneous_cte)
{
  thermal_strain = _thermal_expansion_coeff * (_temperature[_qp] - _stress_free_temperature[_qp]);
  instantaneous_cte = _thermal_expansion_coeff;
}
