//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeMeanThermalExpansionPolynomialEigenstrain.h"

#include "MathUtils.h"

registerMooseObject("TensorMechanicsApp", ADComputeMeanThermalExpansionPolynomialEigenstrain);

InputParameters
ADComputeMeanThermalExpansionPolynomialEigenstrain::validParams()
{
  InputParameters params = ADComputeMeanThermalExpansionEigenstrainBase::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion using a polynomial to "
                             "describe the mean thermal expansion coefficent");
  params.addRequiredParam<std::vector<Real>>(
      "thermal_expansion_coefficients",
      "Coefficients describing the polynimal for the mean thermal expansion as a function of "
      "temperature, starting with the constant term (temperature^0).");
  params.addRequiredParam<Real>("thermal_expansion_function_reference_temperature",
                                "Reference temperature for thermal_exansion_function (IMPORTANT: "
                                "this is different in general from the stress_free_temperature)");

  return params;
}

ADComputeMeanThermalExpansionPolynomialEigenstrain::
    ADComputeMeanThermalExpansionPolynomialEigenstrain(const InputParameters & parameters)
  : ADComputeMeanThermalExpansionEigenstrainBase(parameters),
    _coeffs(getParam<std::vector<Real>>("thermal_expansion_coefficients")),
    _thexp_func_ref_temp(getParam<Real>("thermal_expansion_function_reference_temperature"))
{
}

Real
ADComputeMeanThermalExpansionPolynomialEigenstrain::referenceTemperature()
{
  return _thexp_func_ref_temp;
}

ADReal
ADComputeMeanThermalExpansionPolynomialEigenstrain::meanThermalExpansionCoefficient(
    const ADReal & temperature)
{
  return MathUtils::polynomial(_coeffs, temperature);
}
