//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDilatationThermalExpansionFunctionEigenstrain.h"

#include "Function.h"

registerMooseObject("TensorMechanicsApp", ComputeDilatationThermalExpansionFunctionEigenstrain);
registerMooseObject("TensorMechanicsApp", ADComputeDilatationThermalExpansionFunctionEigenstrain);

template <bool is_ad>
InputParameters
ComputeDilatationThermalExpansionFunctionEigenstrainTempl<is_ad>::validParams()
{
  InputParameters params = ComputeDilatationThermalExpansionEigenstrainBase::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion using a function that "
                             "describes the total dilatation as a function of temperature");
  params.addRequiredParam<FunctionName>(
      "dilatation_function",
      "Function describing the thermal dilatation as a function of temperature");
  return params;
}

template <bool is_ad>
ComputeDilatationThermalExpansionFunctionEigenstrainTempl<is_ad>::
    ComputeDilatationThermalExpansionFunctionEigenstrainTempl(const InputParameters & parameters)
  : ComputeDilatationThermalExpansionEigenstrainBaseTempl<is_ad>(parameters),
    _dilatation_function(this->getFunction("dilatation_function"))
{
}

template <bool is_ad>
ValueAndDerivative<is_ad>
ComputeDilatationThermalExpansionFunctionEigenstrainTempl<is_ad>::computeDilatation(
    const ValueAndDerivative<is_ad> & temperature)
{
  // we need these two branches because we cannot yet evaluate Functions with ChainedReals
  if constexpr (is_ad)
    return _dilatation_function.value(temperature);
  else
    return {_dilatation_function.value(temperature.value()),
            _dilatation_function.timeDerivative(temperature.value()) * temperature.derivatives()};
}

template class ComputeDilatationThermalExpansionFunctionEigenstrainTempl<false>;
template class ComputeDilatationThermalExpansionFunctionEigenstrainTempl<true>;
