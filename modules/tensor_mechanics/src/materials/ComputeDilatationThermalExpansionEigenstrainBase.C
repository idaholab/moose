//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDilatationThermalExpansionEigenstrainBase.h"

InputParameters
ComputeDilatationThermalExpansionEigenstrainBase::validParams()
{
  return ComputeThermalExpansionEigenstrainBase::validParams();
}

ComputeDilatationThermalExpansionEigenstrainBase::ComputeDilatationThermalExpansionEigenstrainBase(
    const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainBase(parameters)
{
}

void
ComputeDilatationThermalExpansionEigenstrainBase::computeThermalStrain(Real & thermal_strain,
                                                                       Real * dthermal_strain_dT)
{
  const Real stress_free_thexp = computeDilatation(_stress_free_temperature[_qp]);
  thermal_strain = computeDilatation(_temperature[_qp]) - stress_free_thexp;

  mooseAssert(dthermal_strain_dT, "Internal error. dthermal_strain_dT should not be nullptr.");
  *dthermal_strain_dT = computeDilatationDerivative(_temperature[_qp]);
}
