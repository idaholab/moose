//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeThermalExpansionEigenstrain.h"

registerMooseObject("TensorMechanicsApp", ComputeThermalExpansionEigenstrain);
registerMooseObject("TensorMechanicsApp", ADComputeThermalExpansionEigenstrain);

template <bool is_ad>
InputParameters
ComputeThermalExpansionEigenstrainTempl<is_ad>::validParams()
{
  InputParameters params = ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion "
                             "with a constant coefficient");
  params.addRequiredParam<Real>("thermal_expansion_coeff", "Thermal expansion coefficient");
  params.declareControllable("thermal_expansion_coeff");

  return params;
}

template <bool is_ad>
ComputeThermalExpansionEigenstrainTempl<is_ad>::ComputeThermalExpansionEigenstrainTempl(
    const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainBaseTempl<is_ad>(parameters),
    _thermal_expansion_coeff(this->template getParam<Real>("thermal_expansion_coeff"))
{
}

template <bool is_ad>
ValueAndDerivative<is_ad>
ComputeThermalExpansionEigenstrainTempl<is_ad>::computeThermalStrain()
{
  return _thermal_expansion_coeff * (_temperature[_qp] - _stress_free_temperature[_qp]);
}

template class ComputeThermalExpansionEigenstrainTempl<false>;
template class ComputeThermalExpansionEigenstrainTempl<true>;
