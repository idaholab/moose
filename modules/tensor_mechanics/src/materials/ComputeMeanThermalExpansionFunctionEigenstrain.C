/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeMeanThermalExpansionFunctionEigenstrain.h"
#include "Function.h"

template <>
InputParameters
validParams<ComputeMeanThermalExpansionFunctionEigenstrain>()
{
  InputParameters params = validParams<ComputeThermalExpansionEigenstrainBase>();
  params.addClassDescription("Computes eigenstrain due to thermal expansion using a function that "
                             "describes the mean thermal expansion as a function of temperature");
  params.addRequiredParam<FunctionName>(
      "thermal_expansion_function",
      "Function describing the mean thermal expansion as a function of temperature");
  params.addRequiredParam<Real>("thermal_expansion_function_reference_temperature",
                                "Reference temperature for thermal_exansion_function (IMPORTANT: "
                                "this is different in general from the stress_free_temperature)");

  return params;
}

ComputeMeanThermalExpansionFunctionEigenstrain::ComputeMeanThermalExpansionFunctionEigenstrain(
    const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainBase(parameters),
    _thermal_expansion_function(getFunction("thermal_expansion_function")),
    _reference_temperature(getParam<Real>("thermal_expansion_function_reference_temperature")),
    _alphabar_stress_free_temperature(0.0),
    _thexp_stress_free_temperature(0.0)
{
}

void
ComputeMeanThermalExpansionFunctionEigenstrain::initialSetup()
{
  Point p;
  _alphabar_stress_free_temperature =
      _thermal_expansion_function.value(_stress_free_temperature, p);
  _thexp_stress_free_temperature =
      _alphabar_stress_free_temperature * (_stress_free_temperature - _reference_temperature);

  // Evaluate the derivative of this function here so it will error out early on if that isn't
  // supported for this function.
  _thermal_expansion_function.timeDerivative(_stress_free_temperature, p);
}

void
ComputeMeanThermalExpansionFunctionEigenstrain::computeThermalStrain(Real & thermal_strain,
                                                                     Real & instantaneous_cte)
{
  const Real small = libMesh::TOLERANCE;
  const Point p;

  const Real & current_temp = _temperature[_qp];
  const Real current_alphabar = _thermal_expansion_function.value(current_temp, p);
  const Real thexp_current_temp = current_alphabar * (current_temp - _reference_temperature);

  // Per the paper:  M. Niffenegger and K. Reichlin. The proper use of thermal expansion
  // coefficients in
  // finite element calculations. Nuclear Engineering and Design, 243:356-359, Feb. 2012,
  // strictly speaking, thermal_strain should be divided by (1.0 + _thexp_stress_free_temperature)
  // to account for the ratio of the length at the stress-free temperature to the length at the
  // reference temperature.
  // While this is very close to 1, we include it for completeness here.

  thermal_strain = (thexp_current_temp - _thexp_stress_free_temperature) /
                   (1.0 + _thexp_stress_free_temperature);

  const Real dalphabar_dT = _thermal_expansion_function.timeDerivative(current_temp, p);
  const Real numerator = dalphabar_dT * (current_temp - _reference_temperature) + current_alphabar;
  const Real denominator =
      1.0 + _alphabar_stress_free_temperature * (_stress_free_temperature - _reference_temperature);
  if (denominator < small)
    mooseError("Denominator too small in thermal strain calculation");
  instantaneous_cte = numerator / denominator;
}
