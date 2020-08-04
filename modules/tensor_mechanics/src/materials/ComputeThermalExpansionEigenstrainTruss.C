//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeThermalExpansionEigenstrainTruss.h"

registerMooseObject("TensorMechanicsApp", ComputeThermalExpansionEigenstrainTruss);

InputParameters
ComputeThermalExpansionEigenstrainTruss::validParams()
{
  InputParameters params = ComputeThermalExpansionEigenstrainTrussBase::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion "
                             "with a constant coefficient");
  params.addParam<Real>("thermal_expansion_coeff", "Thermal expansion coefficient");

  return params;
}

ComputeThermalExpansionEigenstrainTruss::ComputeThermalExpansionEigenstrainTruss(
    const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainTrussBase(parameters),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff"))
{
}

void
ComputeThermalExpansionEigenstrainTruss::computeThermalStrain(Real & thermal_strain)
{
  thermal_strain = _thermal_expansion_coeff * (_temperature[_qp] - _stress_free_temperature[_qp]);
}
