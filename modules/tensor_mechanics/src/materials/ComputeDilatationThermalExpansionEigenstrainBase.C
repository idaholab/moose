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
void
ComputeDilatationThermalExpansionEigenstrainBaseTempl<is_ad>::computeThermalStrain(
    GenericReal<is_ad> & thermal_strain, Real * dthermal_strain_dT)
{
  const auto & T =
      this->_use_old_temperature ? this->_temperature_old[_qp] : this->_temperature[_qp];

  const auto stress_free_thexp = computeDilatation(this->_stress_free_temperature[_qp]);
  thermal_strain = computeDilatation(T) - stress_free_thexp;

  if constexpr (!is_ad)
  {
    mooseAssert(dthermal_strain_dT, "Internal error. dthermal_strain_dT should not be nullptr.");
    *dthermal_strain_dT = computeDilatationDerivative(T);
  }
  else
    libmesh_ignore(dthermal_strain_dT);
}

template <bool is_ad>
Real
ComputeDilatationThermalExpansionEigenstrainBaseTempl<is_ad>::computeDilatationDerivative(
    const Real)
{
  mooseError("computeDilatationDerivative must be implemented for any derived non-AD class.");
}

template class ComputeDilatationThermalExpansionEigenstrainBaseTempl<false>;
template class ComputeDilatationThermalExpansionEigenstrainBaseTempl<true>;
