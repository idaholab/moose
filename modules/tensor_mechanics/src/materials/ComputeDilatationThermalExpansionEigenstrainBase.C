//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDilatationThermalExpansionEigenstrainBase.h"

defineLegacyParams(ComputeDilatationThermalExpansionEigenstrainBase);

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
                                                         Real & dthermal_strain_dT)
{
  const Real stress_free_thexp = computeDilatation(_stress_free_temperature[_qp]);
  thermal_strain = computeDilatation(_temperature[_qp]) - stress_free_thexp;
  dthermal_strain_dT = computeDilatationDerivative(_temperature[_qp]);

  // Per M. Niffenegger and K. Reichlin (2012), thermal_strain should be divided
  // by (1.0 + thexp_stress_free_temperature) to account for the ratio of
  // the length at the stress-free temperature to the length at the reference
  // temperature. It can be neglected because it is very close to 1,
  // but we include it for completeness here.

  thermal_strain /= (1.0 + stress_free_thexp);
  dthermal_strain_dT /= (1.0 + stress_free_thexp);
}
