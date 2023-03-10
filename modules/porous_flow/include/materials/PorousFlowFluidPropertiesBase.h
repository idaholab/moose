//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialBase.h"
#include "PorousFlowDictator.h"

/**
 * Base class for fluid properties materials. All PorousFlow fluid
 * materials must override computeQpProperties()
 */
template <bool is_ad>
class PorousFlowFluidPropertiesBaseTempl : public PorousFlowMaterialBase
{
public:
  static InputParameters validParams();

  PorousFlowFluidPropertiesBaseTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Pore pressure at the nodes or quadpoints
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _porepressure;

  /// Fluid temperature at the nodes or quadpoints
  const GenericMaterialProperty<Real, is_ad> & _temperature;

  /// Conversion from degrees Celsius to degrees Kelvin
  const Real _t_c2k;

  /// Universal gas constant
  const Real _R;

  /// Unit used for porepressure
  const enum class PressureUnitEnum { Pa, MPa } _p_unit;

  /// convert porepressure to Pascals by multiplying by this quantity
  const Real _pressure_to_Pascals;

  /// Unit used for time
  const enum class TimeUnitEnum { seconds, hours, days, years } _time_unit;

  /// convert time to seconds by multiplying by this quantity
  const Real _time_to_seconds;

  /// If true, this Material will compute density and viscosity, and their derivatives
  const bool _compute_rho_mu;

  /// If true, this Material will compute internal energy and its derivatives
  const bool _compute_internal_energy;

  /// If true, this Material will compute enthalpy and its derivatives
  const bool _compute_enthalpy;

  /// Fluid phase density at the qps or nodes
  GenericMaterialProperty<Real, is_ad> * const _density;

  /// Derivative of fluid density wrt phase pore pressure at the qps or nodes
  MaterialProperty<Real> * const _ddensity_dp;

  /// Derivative of fluid density wrt temperature at the qps or nodes
  MaterialProperty<Real> * const _ddensity_dT;

  /// Fluid phase viscosity at the nodes
  GenericMaterialProperty<Real, is_ad> * const _viscosity;

  /// Derivative of fluid phase viscosity wrt pressure at the nodes
  MaterialProperty<Real> * const _dviscosity_dp;

  /// Derivative of fluid phase viscosity wrt temperature at the nodes
  MaterialProperty<Real> * const _dviscosity_dT;

  /// Fluid phase internal_energy at the qps or nodes
  GenericMaterialProperty<Real, is_ad> * const _internal_energy;

  /// Derivative of fluid internal_energy wrt phase pore pressure at the qps or nodes
  MaterialProperty<Real> * const _dinternal_energy_dp;

  /// Derivative of fluid internal_energy wrt temperature at the qps or nodes
  MaterialProperty<Real> * const _dinternal_energy_dT;

  /// Fluid phase enthalpy at the qps or nodes
  GenericMaterialProperty<Real, is_ad> * const _enthalpy;

  /// Derivative of fluid enthalpy wrt phase pore pressure at the qps or nodes
  MaterialProperty<Real> * const _denthalpy_dp;

  /// Derivative of fluid enthalpy wrt temperature at the qps or nodes
  MaterialProperty<Real> * const _denthalpy_dT;
};

#define usingPorousFlowFluidPropertiesMembers                                                      \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_nodal_material;                                \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_phase;                                         \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_phase_num;                                     \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_porepressure;                                  \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_pressure_variable_name;                        \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_qp;                                            \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_t_c2k;                                         \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_temperature;                                   \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_temperature_variable_name;                     \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_mass_fraction_variable_name;                   \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_compute_rho_mu;                                \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_compute_internal_energy;                       \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_compute_enthalpy;                              \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_pressure_to_Pascals;                           \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_time_to_seconds;                               \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_density;                                       \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_ddensity_dp;                                   \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_ddensity_dT;                                   \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_viscosity;                                     \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_dviscosity_dp;                                 \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_dviscosity_dT;                                 \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_internal_energy;                               \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_dinternal_energy_dp;                           \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_dinternal_energy_dT;                           \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_enthalpy;                                      \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_denthalpy_dp;                                  \
  using PorousFlowFluidPropertiesBaseTempl<is_ad>::_denthalpy_dT

typedef PorousFlowFluidPropertiesBaseTempl<false> PorousFlowFluidPropertiesBase;
typedef PorousFlowFluidPropertiesBaseTempl<true> ADPorousFlowFluidPropertiesBase;
