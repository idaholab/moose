//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeMeanThermalExpansionEigenstrainBase.h"

InputParameters
ADComputeMeanThermalExpansionEigenstrainBase::validParams()
{
  InputParameters params = ADComputeThermalExpansionEigenstrainBase::validParams();
  params.addClassDescription("Base class for models that compute eigenstrain due to mean"
                             "thermal expansion as a function of temperature using AD");
  return params;
}

ADComputeMeanThermalExpansionEigenstrainBase::ADComputeMeanThermalExpansionEigenstrainBase(
    const InputParameters & parameters)
  : ADComputeThermalExpansionEigenstrainBase(parameters)
{
}

void
ADComputeMeanThermalExpansionEigenstrainBase::computeThermalStrain(ADReal & thermal_strain, Real *)
{
  const Real reference_temperature = referenceTemperature();
  const ADReal & current_temp = _temperature[_qp];
  const ADReal current_alphabar = meanThermalExpansionCoefficient(current_temp);
  const ADReal thexp_current_temp = current_alphabar * (current_temp - reference_temperature);

  // Mean linear thermal expansion coefficient relative to the reference temperature
  // evaluated at stress_free_temperature.  This is
  // \f$\bar{\alpha} = (\delta L(T_{sf}) / L) / (T_{sf} - T_{ref})\f$
  // where \f$T_sf\f$ is the stress-free temperature and \f$T_{ref}\f$ is the reference temperature.
  const ADReal alphabar_stress_free_temperature =
      meanThermalExpansionCoefficient(_stress_free_temperature[_qp]);
  // Thermal expansion relative to the reference temperature evaluated at stress_free_temperature
  // \f$(\delta L(T_sf) / L)\f$, where \f$T_sf\f$ is the stress-free temperature.
  const ADReal thexp_stress_free_temperature =
      alphabar_stress_free_temperature * (_stress_free_temperature[_qp] - referenceTemperature());

  // Per M. Niffenegger and K. Reichlin (2012), thermal_strain should be divided
  // by (1.0 + thexp_stress_free_temperature) to account for the ratio of
  // the length at the stress-free temperature to the length at the reference
  // temperature. It can be neglected because it is very close to 1,
  // but we include it for completeness here.

  thermal_strain =
      (thexp_current_temp - thexp_stress_free_temperature) / (1.0 + thexp_stress_free_temperature);
}
