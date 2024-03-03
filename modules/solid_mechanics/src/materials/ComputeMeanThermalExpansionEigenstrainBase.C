//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMeanThermalExpansionEigenstrainBase.h"
#include "Function.h"

template <bool is_ad>
InputParameters
ComputeMeanThermalExpansionEigenstrainBaseTempl<is_ad>::validParams()
{
  InputParameters params = ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::validParams();
  params.addClassDescription("Base class for models that compute eigenstrain due to mean"
                             "thermal expansion as a function of temperature");
  params.addParam<Real>("thermal_expansion_scale_factor",
                        1.0,
                        "Scaling factor on the thermal expansion strain. This input parameter can "
                        "be used to perform sensitivity analysis on thermal expansion.");
  params.addParamNamesToGroup("thermal_expansion_scale_factor", "Advanced");

  return params;
}

template <bool is_ad>
ComputeMeanThermalExpansionEigenstrainBaseTempl<
    is_ad>::ComputeMeanThermalExpansionEigenstrainBaseTempl(const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainBaseTempl<is_ad>(parameters),
    _thermal_expansion_scale_factor(this->template getParam<Real>("thermal_expansion_scale_factor"))
{
}

template <bool is_ad>
ValueAndDerivative<is_ad>
ComputeMeanThermalExpansionEigenstrainBaseTempl<is_ad>::computeThermalStrain()
{
  const auto reference_temperature = referenceTemperature();

  const auto current_alphabar = meanThermalExpansionCoefficient(this->_temperature[_qp]);
  const auto thexp_T = current_alphabar * (this->_temperature[_qp] - reference_temperature);

  // Mean linear thermal expansion coefficient relative to the reference temperature
  // evaluated at stress_free_temperature.  This is
  // \f$\bar{\alpha} = (\delta L(T_{sf}) / L) / (T_{sf} - T_{ref})\f$
  // where \f$T_sf\f$ is the stress-free temperature and \f$T_{ref}\f$ is the reference temperature.
  const auto alphabar_stress_free_temperature =
      meanThermalExpansionCoefficient(this->_stress_free_temperature[_qp]);

  // Thermal expansion relative to the reference temperature evaluated at stress_free_temperature
  // \f$(\delta L(T_sf) / L)\f$, where \f$T_sf\f$ is the stress-free temperature.
  const auto thexp_stress_free_temperature =
      alphabar_stress_free_temperature *
      (this->_stress_free_temperature[_qp] - referenceTemperature());

  // Per M. Niffenegger and K. Reichlin (2012), thermal_strain should be divided
  // by (1.0 + thexp_stress_free_temperature) to account for the ratio of
  // the length at the stress-free temperature to the length at the reference
  // temperature. It can be neglected because it is very close to 1,
  // but we include it for completeness here.

  auto thermal_strain =
      (thexp_T - thexp_stress_free_temperature) / (1.0 + thexp_stress_free_temperature);

  return _thermal_expansion_scale_factor * thermal_strain;
}

template class ComputeMeanThermalExpansionEigenstrainBaseTempl<false>;
template class ComputeMeanThermalExpansionEigenstrainBaseTempl<true>;
