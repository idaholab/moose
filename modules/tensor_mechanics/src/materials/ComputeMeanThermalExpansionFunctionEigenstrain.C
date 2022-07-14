//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMeanThermalExpansionFunctionEigenstrain.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", ComputeMeanThermalExpansionFunctionEigenstrain);
registerMooseObject("TensorMechanicsApp", ADComputeMeanThermalExpansionFunctionEigenstrain);

template <bool is_ad>
InputParameters
ComputeMeanThermalExpansionFunctionEigenstrainTempl<is_ad>::validParams()
{
  InputParameters params = ComputeMeanThermalExpansionEigenstrainBaseTempl<is_ad>::validParams();
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

template <bool is_ad>
ComputeMeanThermalExpansionFunctionEigenstrainTempl<
    is_ad>::ComputeMeanThermalExpansionFunctionEigenstrainTempl(const InputParameters & parameters)
  : ComputeMeanThermalExpansionEigenstrainBaseTempl<is_ad>(parameters),
    _thermal_expansion_function(this->getFunction("thermal_expansion_function")),
    _thexp_func_ref_temp(
        this->template getParam<Real>("thermal_expansion_function_reference_temperature"))
{
}

template <bool is_ad>
Real
ComputeMeanThermalExpansionFunctionEigenstrainTempl<is_ad>::referenceTemperature()
{
  return _thexp_func_ref_temp;
}

template <bool is_ad>
ValueAndDerivative<is_ad>
ComputeMeanThermalExpansionFunctionEigenstrainTempl<is_ad>::meanThermalExpansionCoefficient(
    const ValueAndDerivative<is_ad> & temperature)
{
  // we need these two branches because we cannot yet evaluate Functions with ChainedReals
  if constexpr (is_ad)
    return _thermal_expansion_function.value(temperature);
  else
    return {_thermal_expansion_function.value(temperature.value()),
            _thermal_expansion_function.timeDerivative(temperature.value()) *
                temperature.derivatives()};
}

template class ComputeMeanThermalExpansionFunctionEigenstrainTempl<false>;
template class ComputeMeanThermalExpansionFunctionEigenstrainTempl<true>;
