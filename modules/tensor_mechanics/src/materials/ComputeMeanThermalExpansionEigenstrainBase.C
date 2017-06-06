/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeMeanThermalExpansionEigenstrainBase.h"
#include "Function.h"

template <>
InputParameters
validParams<ComputeMeanThermalExpansionEigenstrainBase>()
{
  InputParameters params = validParams<ComputeThermalExpansionEigenstrainBase>();
  params.addClassDescription("Base class for models that compute eigenstrain due to mean"
                             "thermal expansion as a function of temperature");
  return params;
}

ComputeMeanThermalExpansionEigenstrainBase::ComputeMeanThermalExpansionEigenstrainBase(
    const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainBase(parameters),
    _alphabar_stress_free_temperature(0.0),
    _thexp_stress_free_temperature(0.0)
{
}

void
ComputeMeanThermalExpansionEigenstrainBase::initialSetup()
{
  _alphabar_stress_free_temperature = meanThermalExpansion(_stress_free_temperature);
  _thexp_stress_free_temperature =
      _alphabar_stress_free_temperature * (_stress_free_temperature - referenceTemperature());

  // Evaluate the derivative so it will error out early on if there are any issues with that
  meanThermalExpansionDerivative(_stress_free_temperature);
}

void
ComputeMeanThermalExpansionEigenstrainBase::computeThermalStrain(Real & thermal_strain,
                                                                 Real & instantaneous_cte)
{
  const Real small = libMesh::TOLERANCE;

  const Real reference_temperature = referenceTemperature();
  const Real & current_temp = _temperature[_qp];
  const Real current_alphabar = meanThermalExpansion(current_temp);
  const Real thexp_current_temp = current_alphabar * (current_temp - reference_temperature);

  // Per the paper:  M. Niffenegger and K. Reichlin. The proper use of thermal expansion
  // coefficients in
  // finite element calculations. Nuclear Engineering and Design, 243:356-359, Feb. 2012,
  // strictly speaking, thermal_strain should be divided by (1.0 + _thexp_stress_free_temperature)
  // to account for the ratio of the length at the stress-free temperature to the length at the
  // reference temperature.
  // While this is very close to 1, we include it for completeness here.

  thermal_strain = (thexp_current_temp - _thexp_stress_free_temperature) /
                   (1.0 + _thexp_stress_free_temperature);

  const Real dalphabar_dT = meanThermalExpansionDerivative(current_temp);
  const Real numerator = dalphabar_dT * (current_temp - reference_temperature) + current_alphabar;
  const Real denominator =
      1.0 + _alphabar_stress_free_temperature * (_stress_free_temperature - reference_temperature);
  if (denominator < small)
    mooseError("Denominator too small in thermal strain calculation");
  instantaneous_cte = numerator / denominator;
}
