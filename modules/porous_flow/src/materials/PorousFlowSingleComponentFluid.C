//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowSingleComponentFluid.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("PorousFlowApp", PorousFlowSingleComponentFluid);
registerMooseObject("PorousFlowApp", ADPorousFlowSingleComponentFluid);

template <bool is_ad>
InputParameters
PorousFlowSingleComponentFluidTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowFluidPropertiesBase::validParams();
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription("This Material calculates fluid properties at the quadpoints or nodes "
                             "for a single component fluid");
  return params;
}

template <bool is_ad>
PorousFlowSingleComponentFluidTempl<is_ad>::PorousFlowSingleComponentFluidTempl(
    const InputParameters & parameters)
  : PorousFlowFluidPropertiesBaseTempl<is_ad>(parameters),
    _p_unit(
        this->template getParam<MooseEnum>("pressure_unit").template getEnum<PressureUnitEnum>()),
    _pressure_to_Pascals(_p_unit == PressureUnitEnum::Pa ? 1.0 : 1.0E6),
    _time_unit(this->template getParam<MooseEnum>("time_unit").template getEnum<TimeUnitEnum>()),
    _time_to_seconds(_time_unit == TimeUnitEnum::seconds ? 1.0
                     : _time_unit == TimeUnitEnum::hours ? 3600.0
                     : _time_unit == TimeUnitEnum::days  ? 3600.0 * 24
                                                         : 3600 * 24 * 365.25),
    _compute_rho_mu(this->template getParam<bool>("compute_density_and_viscosity")),
    _compute_internal_energy(this->template getParam<bool>("compute_internal_energy")),
    _compute_enthalpy(this->template getParam<bool>("compute_enthalpy")),
    _density(_compute_rho_mu
                 ? (_nodal_material ? &this->template declareGenericProperty<Real, is_ad>(
                                          "PorousFlow_fluid_phase_density_nodal" + _phase)
                                    : &this->template declareGenericProperty<Real, is_ad>(
                                          "PorousFlow_fluid_phase_density_qp" + _phase))
                 : nullptr),
    _ddensity_dp(
        (_compute_rho_mu && !is_ad)
            ? (_nodal_material
                   ? &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_fluid_phase_density_nodal" + _phase, _pressure_variable_name)
                   : &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_fluid_phase_density_qp" + _phase, _pressure_variable_name))
            : nullptr),
    _ddensity_dT((_compute_rho_mu && !is_ad)
                     ? (_nodal_material ? &this->template declarePropertyDerivative<Real>(
                                              "PorousFlow_fluid_phase_density_nodal" + _phase,
                                              _temperature_variable_name)
                                        : &this->template declarePropertyDerivative<Real>(
                                              "PorousFlow_fluid_phase_density_qp" + _phase,
                                              _temperature_variable_name))
                     : nullptr),

    _viscosity(_compute_rho_mu
                   ? (_nodal_material ? &this->template declareGenericProperty<Real, is_ad>(
                                            "PorousFlow_viscosity_nodal" + _phase)
                                      : &this->template declareGenericProperty<Real, is_ad>(
                                            "PorousFlow_viscosity_qp" + _phase))
                   : nullptr),
    _dviscosity_dp((_compute_rho_mu && !is_ad)
                       ? (_nodal_material
                              ? &this->template declarePropertyDerivative<Real>(
                                    "PorousFlow_viscosity_nodal" + _phase, _pressure_variable_name)
                              : &this->template declarePropertyDerivative<Real>(
                                    "PorousFlow_viscosity_qp" + _phase, _pressure_variable_name))
                       : nullptr),
    _dviscosity_dT(
        (_compute_rho_mu && !is_ad)
            ? (_nodal_material
                   ? &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_viscosity_nodal" + _phase, _temperature_variable_name)
                   : &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_viscosity_qp" + _phase, _temperature_variable_name))
            : nullptr),

    _internal_energy(_compute_internal_energy
                         ? (_nodal_material
                                ? &this->template declareGenericProperty<Real, is_ad>(
                                      "PorousFlow_fluid_phase_internal_energy_nodal" + _phase)
                                : &this->template declareGenericProperty<Real, is_ad>(
                                      "PorousFlow_fluid_phase_internal_energy_qp" + _phase))
                         : nullptr),
    _dinternal_energy_dp((_compute_internal_energy && !is_ad)
                             ? (_nodal_material
                                    ? &this->template declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_nodal" + _phase,
                                          _pressure_variable_name)
                                    : &this->template declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                          _pressure_variable_name))
                             : nullptr),
    _dinternal_energy_dT((_compute_internal_energy && !is_ad)
                             ? (_nodal_material
                                    ? &this->template declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_nodal" + _phase,
                                          _temperature_variable_name)
                                    : &this->template declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                          _temperature_variable_name))
                             : nullptr),

    _enthalpy(_compute_enthalpy
                  ? (_nodal_material ? &this->template declareGenericProperty<Real, is_ad>(
                                           "PorousFlow_fluid_phase_enthalpy_nodal" + _phase)
                                     : &this->template declareGenericProperty<Real, is_ad>(
                                           "PorousFlow_fluid_phase_enthalpy_qp" + _phase))
                  : nullptr),
    _denthalpy_dp(
        (_compute_enthalpy && !is_ad)
            ? (_nodal_material
                   ? &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_fluid_phase_enthalpy_nodal" + _phase, _pressure_variable_name)
                   : &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_fluid_phase_enthalpy_qp" + _phase, _pressure_variable_name))
            : nullptr),
    _denthalpy_dT((_compute_enthalpy && !is_ad)
                      ? (_nodal_material ? &this->template declarePropertyDerivative<Real>(
                                               "PorousFlow_fluid_phase_enthalpy_nodal" + _phase,
                                               _temperature_variable_name)
                                         : &this->template declarePropertyDerivative<Real>(
                                               "PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                               _temperature_variable_name))
                      : nullptr),

    _fp(this->template getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

template <bool is_ad>
void
PorousFlowSingleComponentFluidTempl<is_ad>::initQpStatefulProperties()
{
  if (_compute_rho_mu)
  {
    (*_density)[_qp] = _fp.rho_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                        _temperature[_qp] + _t_c2k);

    (*_viscosity)[_qp] = _fp.mu_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                         _temperature[_qp] + _t_c2k) /
                         _pressure_to_Pascals / _time_to_seconds;
  }

  if (_compute_internal_energy)
    (*_internal_energy)[_qp] = _fp.e_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                              _temperature[_qp] + _t_c2k);
  if (_compute_enthalpy)
    (*_enthalpy)[_qp] = _fp.h_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                       _temperature[_qp] + _t_c2k);
}

template <bool is_ad>
void
PorousFlowSingleComponentFluidTempl<is_ad>::computeQpProperties()
{
  if (_compute_rho_mu)
  {
    if (is_ad)
    {
      GenericReal<is_ad> rho, mu;
      _fp.rho_mu_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                          _temperature[_qp] + _t_c2k,
                          rho,
                          mu);

      (*_density)[_qp] = rho;
      (*_viscosity)[_qp] = mu / _pressure_to_Pascals / _time_to_seconds;
    }
    else
    {
      // Density and viscosity, and derivatives wrt pressure and temperature
      Real rho, drho_dp, drho_dT, mu, dmu_dp, dmu_dT;
      _fp.rho_mu_from_p_T(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                              _pressure_to_Pascals,
                          MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                          rho,
                          drho_dp,
                          drho_dT,
                          mu,
                          dmu_dp,
                          dmu_dT);
      (*_density)[_qp] = rho;
      (*_ddensity_dp)[_qp] = drho_dp * _pressure_to_Pascals;
      (*_ddensity_dT)[_qp] = drho_dT;
      (*_viscosity)[_qp] = mu / _pressure_to_Pascals / _time_to_seconds;
      (*_dviscosity_dp)[_qp] = dmu_dp / _time_to_seconds;
      (*_dviscosity_dT)[_qp] = dmu_dT / _pressure_to_Pascals / _time_to_seconds;
    }
  }

  if (_compute_internal_energy)
  {
    if (is_ad)
      (*_internal_energy)[_qp] = _fp.e_from_p_T(
          _porepressure[_qp][_phase_num] * _pressure_to_Pascals, _temperature[_qp] + _t_c2k);
    else
    {
      // Internal energy and derivatives wrt pressure and temperature at the qps
      Real e, de_dp, de_dT;
      _fp.e_from_p_T(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) * _pressure_to_Pascals,
                     MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                     e,
                     de_dp,
                     de_dT);
      (*_internal_energy)[_qp] = e;
      (*_dinternal_energy_dp)[_qp] = de_dp * _pressure_to_Pascals;
      (*_dinternal_energy_dT)[_qp] = de_dT;
    }
  }

  if (_compute_enthalpy)
  {
    if (is_ad)
      (*_enthalpy)[_qp] = _fp.h_from_p_T(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                         _temperature[_qp] + _t_c2k);
    else
    {
      // Enthalpy and derivatives wrt pressure and temperature at the qps
      Real h, dh_dp, dh_dT;
      _fp.h_from_p_T(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) * _pressure_to_Pascals,
                     MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                     h,
                     dh_dp,
                     dh_dT);
      (*_enthalpy)[_qp] = h;
      (*_denthalpy_dp)[_qp] = dh_dp * _pressure_to_Pascals;
      (*_denthalpy_dT)[_qp] = dh_dT;
    }
  }
}

template class PorousFlowSingleComponentFluidTempl<false>;
template class PorousFlowSingleComponentFluidTempl<true>;
