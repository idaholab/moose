//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMultiComponentFluid.h"
#include "MultiComponentFluidProperties.h"

registerMooseObject("PorousFlowApp", PorousFlowMultiComponentFluid);
registerMooseObject("PorousFlowApp", ADPorousFlowMultiComponentFluid);

template <bool is_ad>
InputParameters
PorousFlowMultiComponentFluidTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowFluidPropertiesBaseTempl<is_ad>::validParams();
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addCoupledVar("x", 0, "The mass fraction variable");
  params.addClassDescription(
      "This Material calculates fluid properties for a multicomponent fluid");
  return params;
}

template <bool is_ad>
PorousFlowMultiComponentFluidTempl<is_ad>::PorousFlowMultiComponentFluidTempl(
    const InputParameters & parameters)
  : PorousFlowFluidPropertiesBaseTempl<is_ad>(parameters),
    _ddensity_dX(_compute_rho_mu
                     ? (_nodal_material ? &this->template declarePropertyDerivative<Real>(
                                              "PorousFlow_fluid_phase_density_nodal" + _phase,
                                              _mass_fraction_variable_name)
                                        : &this->template declarePropertyDerivative<Real>(
                                              "PorousFlow_fluid_phase_density_qp" + _phase,
                                              _mass_fraction_variable_name))
                     : nullptr),
    _dviscosity_dX(
        _compute_rho_mu
            ? (_nodal_material
                   ? &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_viscosity_nodal" + _phase, _mass_fraction_variable_name)
                   : &this->template declarePropertyDerivative<Real>(
                         "PorousFlow_viscosity_qp" + _phase, _mass_fraction_variable_name))
            : nullptr),
    _dinternal_energy_dX(_compute_internal_energy
                             ? (_nodal_material
                                    ? &this->template declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_nodal" + _phase,
                                          _mass_fraction_variable_name)
                                    : &this->template declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                          _mass_fraction_variable_name))
                             : nullptr),
    _denthalpy_dX(_compute_enthalpy
                      ? (_nodal_material ? &this->template declarePropertyDerivative<Real>(
                                               "PorousFlow_fluid_phase_enthalpy_nodal" + _phase,
                                               _mass_fraction_variable_name)
                                         : &this->template declarePropertyDerivative<Real>(
                                               "PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                               _mass_fraction_variable_name))
                      : nullptr),
    _fp(this->template getUserObject<MultiComponentFluidProperties>("fp")),
    _is_X_nodal(isCoupled("x") ? getVar("x", 0)->isNodal() : false),
    _X(_nodal_material && _is_X_nodal ? this->template coupledGenericDofValue<is_ad>("x")
                                      : this->template coupledGenericValue<is_ad>("x"))
{
}

template <bool is_ad>
void
PorousFlowMultiComponentFluidTempl<is_ad>::initQpStatefulProperties()
{
  if (_compute_rho_mu)
    (*_density)[_qp] = _fp.rho_from_p_T_X(
        _porepressure[_qp][_phase_num] * _pressure_to_Pascals, _temperature[_qp] + _t_c2k, _X[_qp]);

  if (_compute_internal_energy)
    (*_internal_energy)[_qp] = _fp.e_from_p_T_X(
        _porepressure[_qp][_phase_num] * _pressure_to_Pascals, _temperature[_qp] + _t_c2k, _X[_qp]);

  if (_compute_enthalpy)
    (*_enthalpy)[_qp] = _fp.h_from_p_T_X(
        _porepressure[_qp][_phase_num] * _pressure_to_Pascals, _temperature[_qp] + _t_c2k, _X[_qp]);
}

template <bool is_ad>
void
PorousFlowMultiComponentFluidTempl<is_ad>::computeQpProperties()
{
  const GenericReal<is_ad> Tk = _temperature[_qp] + _t_c2k;

  if (_compute_rho_mu)
  {
    if (is_ad)
    {
      (*_density)[_qp] =
          _fp.rho_from_p_T_X(_porepressure[_qp][_phase_num] * _pressure_to_Pascals, Tk, _X[_qp]);
      (*_viscosity)[_qp] =
          _fp.mu_from_p_T_X(_porepressure[_qp][_phase_num] * _pressure_to_Pascals, Tk, _X[_qp]) /
          _pressure_to_Pascals / _time_to_seconds;
    }
    else
    {
      // Density and derivatives wrt pressure and temperature
      Real rho, drho_dp, drho_dT, drho_dx;
      _fp.rho_from_p_T_X(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                             _pressure_to_Pascals,
                         MetaPhysicL::raw_value(Tk),
                         MetaPhysicL::raw_value(_X[_qp]),
                         rho,
                         drho_dp,
                         drho_dT,
                         drho_dx);
      (*_density)[_qp] = rho;
      (*_ddensity_dp)[_qp] = drho_dp * _pressure_to_Pascals;
      (*_ddensity_dT)[_qp] = drho_dT;
      (*_ddensity_dX)[_qp] = drho_dx;

      // Viscosity and derivatives wrt pressure and temperature
      Real mu, dmu_dp, dmu_dT, dmu_dx;
      _fp.mu_from_p_T_X(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                            _pressure_to_Pascals,
                        MetaPhysicL::raw_value(Tk),
                        MetaPhysicL::raw_value(_X[_qp]),
                        mu,
                        dmu_dp,
                        dmu_dT,
                        dmu_dx);
      (*_viscosity)[_qp] = mu / _pressure_to_Pascals / _time_to_seconds;
      (*_dviscosity_dp)[_qp] = dmu_dp / _time_to_seconds;
      (*_dviscosity_dT)[_qp] = dmu_dT / _pressure_to_Pascals / _time_to_seconds;
      (*_dviscosity_dX)[_qp] = dmu_dx / _pressure_to_Pascals / _time_to_seconds;
    }
  }

  // Internal energy and derivatives wrt pressure and temperature
  if (_compute_internal_energy)
  {
    if (is_ad)
      (*_internal_energy)[_qp] =
          _fp.e_from_p_T_X(_porepressure[_qp][_phase_num] * _pressure_to_Pascals, Tk, _X[_qp]);
    else
    {
      Real e, de_dp, de_dT, de_dx;
      _fp.e_from_p_T_X(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                           _pressure_to_Pascals,
                       MetaPhysicL::raw_value(Tk),
                       MetaPhysicL::raw_value(_X[_qp]),
                       e,
                       de_dp,
                       de_dT,
                       de_dx);
      (*_internal_energy)[_qp] = e;
      (*_dinternal_energy_dp)[_qp] = de_dp * _pressure_to_Pascals;
      (*_dinternal_energy_dT)[_qp] = de_dT;
      (*_dinternal_energy_dX)[_qp] = de_dx;
    }
  }

  // Enthalpy and derivatives wrt pressure and temperature
  if (_compute_enthalpy)
  {
    if (is_ad)
      (*_enthalpy)[_qp] =
          _fp.h_from_p_T_X(_porepressure[_qp][_phase_num] * _pressure_to_Pascals, Tk, _X[_qp]);
    else
    {
      Real h, dh_dp, dh_dT, dh_dx;
      _fp.h_from_p_T_X(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                           _pressure_to_Pascals,
                       MetaPhysicL::raw_value(Tk),
                       MetaPhysicL::raw_value(_X[_qp]),
                       h,
                       dh_dp,
                       dh_dT,
                       dh_dx);
      (*_enthalpy)[_qp] = h;
      (*_denthalpy_dp)[_qp] = dh_dp * _pressure_to_Pascals;
      (*_denthalpy_dT)[_qp] = dh_dT;
      (*_denthalpy_dX)[_qp] = dh_dx;
    }
  }
}

template class PorousFlowMultiComponentFluidTempl<false>;
template class PorousFlowMultiComponentFluidTempl<true>;
