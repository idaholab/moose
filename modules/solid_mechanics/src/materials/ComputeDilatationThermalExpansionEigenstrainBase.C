//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDilatationThermalExpansionEigenstrainBase.h"

template <bool is_ad>
InputParameters
ComputeDilatationThermalExpansionEigenstrainBaseTempl<is_ad>::validParams()
{
  return ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::validParams();
}

template <bool is_ad>
ComputeDilatationThermalExpansionEigenstrainBaseTempl<is_ad>::
    ComputeDilatationThermalExpansionEigenstrainBaseTempl(const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainBaseTempl<is_ad>(parameters)
{
}

template <bool is_ad>
ValueAndDerivative<is_ad>
ComputeDilatationThermalExpansionEigenstrainBaseTempl<is_ad>::computeThermalStrain()
{
  const auto stress_free = computeDilatation(this->_stress_free_temperature[_qp]);
  const auto current = computeDilatation(this->_temperature[_qp]);

  // in non-AD mode the T derivative of the stress_free term needs get dropped.
  // We assume _stress_free_temperature does not depend on T. In AD mode this is automatic.
  return current - (is_ad ? stress_free : stress_free.value());
}

template class ComputeDilatationThermalExpansionEigenstrainBaseTempl<false>;
template class ComputeDilatationThermalExpansionEigenstrainBaseTempl<true>;
