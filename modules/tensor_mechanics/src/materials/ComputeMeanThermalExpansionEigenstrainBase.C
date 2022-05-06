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
  return params;
}

template <bool is_ad>
ComputeMeanThermalExpansionEigenstrainBaseTempl<
    is_ad>::ComputeMeanThermalExpansionEigenstrainBaseTempl(const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainBaseTempl<is_ad>(parameters)
{
}

template <bool is_ad>
void
ComputeMeanThermalExpansionEigenstrainBaseTempl<is_ad>::computeThermalStrain(
    GenericReal<is_ad> & thermal_strain, Real * dthermal_strain_dT)
{
  const Real small = libMesh::TOLERANCE;

  const auto reference_temperature = referenceTemperature();

  const auto & T =
      this->_use_old_temperature ? this->_temperature_old[_qp] : this->_temperature[_qp];

  const auto current_alphabar = meanThermalExpansionCoefficient(T);
  const auto thexp_T = current_alphabar * (T - reference_temperature);

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

  thermal_strain =
      (thexp_T - thexp_stress_free_temperature) / (1.0 + thexp_stress_free_temperature);

  if constexpr (!is_ad)
  {
    const Real dalphabar_dT = meanThermalExpansionCoefficientDerivative(T);
    const Real numerator = dalphabar_dT * (T - reference_temperature) + current_alphabar;
    const Real denominator =
        1.0 + alphabar_stress_free_temperature *
                  (this->_stress_free_temperature[_qp] - reference_temperature);
    if (denominator < small)
      mooseError("Denominator too small in thermal strain calculation");

    mooseAssert(dthermal_strain_dT, "Internal error. dthermal_strain_dT should not be nullptr.");
    *dthermal_strain_dT = numerator / denominator;
  }
  else
    libmesh_ignore(dthermal_strain_dT);
}

template <bool is_ad>
Real
ComputeMeanThermalExpansionEigenstrainBaseTempl<is_ad>::meanThermalExpansionCoefficientDerivative(
    const Real)
{
  mooseError("meanThermalExpansionCoefficientDerivative must be implemented for any derived non-AD "
             "class.");
}

template class ComputeMeanThermalExpansionEigenstrainBaseTempl<false>;
template class ComputeMeanThermalExpansionEigenstrainBaseTempl<true>;
