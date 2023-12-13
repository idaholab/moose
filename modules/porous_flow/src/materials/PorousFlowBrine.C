//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BrineFluidProperties.h"
#include "PorousFlowBrine.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("PorousFlowApp", PorousFlowBrine);
registerMooseObject("PorousFlowApp", ADPorousFlowBrine);

template <bool is_ad>
InputParameters
PorousFlowBrineTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMultiComponentFluidBaseTempl<is_ad>::validParams();
  params.addParam<UserObjectName>("water_fp",
                                  "The name of the FluidProperties UserObject for water");
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  params.addClassDescription(
      "This Material calculates fluid properties for brine at the quadpoints or nodes");
  return params;
}

template <bool is_ad>
PorousFlowBrineTempl<is_ad>::PorousFlowBrineTempl(const InputParameters & parameters)
  : PorousFlowMultiComponentFluidBaseTempl<is_ad>(parameters),
    _is_xnacl_nodal(isCoupled("xnacl") ? getFieldVar("xnacl", 0)->isNodal() : false),
    _xnacl(_nodal_material && _is_xnacl_nodal
               ? this->template coupledGenericDofValue<is_ad>("xnacl")
               : this->template coupledGenericValue<is_ad>("xnacl")),
    _is_xnacl_pfvar(_dictator.isPorousFlowVariable(coupled("xnacl")))
{
  if (parameters.isParamSetByUser("water_fp"))
  {
    _water_fp = &this->template getUserObject<SinglePhaseFluidProperties>("water_fp");
    // Check that a water userobject has actually been supplied
    if (_water_fp->fluidName() != "water")
      mooseError("water_fp", "A water FluidProperties UserObject must be supplied");
  }

  // BrineFluidProperties UserObject
  const std::string brine_name = name() + ":brine";
  {
    const std::string class_name = "BrineFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);

    if (parameters.isParamSetByUser("water_fp"))
      params.set<UserObjectName>("water_fp") = _water_fp->name();

    if (_tid == 0)
      _fe_problem.addUserObject(class_name, brine_name, params);
  }
  _brine_fp = &_fe_problem.template getUserObject<BrineFluidProperties>(brine_name);
}

template <bool is_ad>
void
PorousFlowBrineTempl<is_ad>::initQpStatefulProperties()
{
  if (_compute_rho_mu)
    (*_density)[_qp] =
        _brine_fp->rho_from_p_T_X(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                  _temperature[_qp] + _t_c2k,
                                  _xnacl[_qp]);

  if (_compute_internal_energy)
    (*_internal_energy)[_qp] = _brine_fp->e_from_p_T_X(
        MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) * _pressure_to_Pascals,
        MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
        MetaPhysicL::raw_value(_xnacl[_qp]));
  if (_compute_enthalpy)
    (*_enthalpy)[_qp] =
        _brine_fp->h_from_p_T_X(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                _temperature[_qp] + _t_c2k,
                                _xnacl[_qp]);
}

template <bool is_ad>
void
PorousFlowBrineTempl<is_ad>::computeQpProperties()
{
  // Density and derivatives wrt pressure and temperature
  if (_compute_rho_mu)
  {
    if (is_ad)
    {
      (*_density)[_qp] =
          _brine_fp->rho_from_p_T_X(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                    _temperature[_qp] + _t_c2k,
                                    _xnacl[_qp]);
      (*_viscosity)[_qp] = _brine_fp->mu_from_p_T_X(_porepressure[_qp][_phase_num],
                                                    _temperature[_qp] + _t_c2k,
                                                    _xnacl[_qp]) /
                           _pressure_to_Pascals / _time_to_seconds;
    }
    else
    {
      Real rho, drho_dp, drho_dT, drho_dx;
      (*_ddensity_dX)[_qp].resize(1);

      _brine_fp->rho_from_p_T_X(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                                    _pressure_to_Pascals,
                                MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                                MetaPhysicL::raw_value(_xnacl[_qp]),
                                rho,
                                drho_dp,
                                drho_dT,
                                drho_dx);
      (*_density)[_qp] = rho;
      (*_ddensity_dp)[_qp] = drho_dp * _pressure_to_Pascals;
      (*_ddensity_dT)[_qp] = drho_dT;
      if (_is_xnacl_pfvar)
        (*_ddensity_dX)[_qp][0] = drho_dx;

      // Viscosity and derivatives wrt pressure and temperature
      Real mu, dmu_dp, dmu_dT, dmu_dx;
      (*_dviscosity_dX)[_qp].resize(1);

      _brine_fp->mu_from_p_T_X(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                                   _pressure_to_Pascals,
                               MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                               MetaPhysicL::raw_value(_xnacl[_qp]),
                               mu,
                               dmu_dp,
                               dmu_dT,
                               dmu_dx);
      (*_viscosity)[_qp] = mu / _pressure_to_Pascals / _time_to_seconds;
      (*_dviscosity_dp)[_qp] = dmu_dp / _time_to_seconds;
      (*_dviscosity_dT)[_qp] = dmu_dT / _pressure_to_Pascals / _time_to_seconds;
      if (_is_xnacl_pfvar)
        (*_dviscosity_dX)[_qp][0] = dmu_dx / _pressure_to_Pascals / _time_to_seconds;
    }
  }

  // Internal energy and derivatives wrt pressure and temperature
  if (_compute_internal_energy)
  {
    if (is_ad)
      (*_internal_energy)[_qp] =
          _brine_fp->e_from_p_T_X(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]),
                                  MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                                  MetaPhysicL::raw_value(_xnacl[_qp]));
    else
    {
      Real e, de_dp, de_dT, de_dx;
      (*_dinternal_energy_dX)[_qp].resize(1);
      _brine_fp->e_from_p_T_X(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                                  _pressure_to_Pascals,
                              MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                              MetaPhysicL::raw_value(_xnacl[_qp]),
                              e,
                              de_dp,
                              de_dT,
                              de_dx);
      (*_internal_energy)[_qp] = e;
      (*_dinternal_energy_dp)[_qp] = de_dp * _pressure_to_Pascals;
      (*_dinternal_energy_dT)[_qp] = de_dT;
      if (_is_xnacl_pfvar)
        (*_dinternal_energy_dX)[_qp][0] = de_dx;
    }
  }

  // Enthalpy and derivatives wrt pressure and temperature
  if (_compute_enthalpy)
  {
    if (is_ad)
      (*_enthalpy)[_qp] =
          _brine_fp->h_from_p_T_X(_porepressure[_qp][_phase_num] * _pressure_to_Pascals,
                                  _temperature[_qp] + _t_c2k,
                                  _xnacl[_qp]);
    else
    {
      Real h, dh_dp, dh_dT, dh_dx;
      (*_denthalpy_dX)[_qp].resize(1);
      _brine_fp->h_from_p_T_X(MetaPhysicL::raw_value(_porepressure[_qp][_phase_num]) *
                                  _pressure_to_Pascals,
                              MetaPhysicL::raw_value(_temperature[_qp]) + _t_c2k,
                              MetaPhysicL::raw_value(_xnacl[_qp]),
                              h,
                              dh_dp,
                              dh_dT,
                              dh_dx);
      (*_enthalpy)[_qp] = h;
      (*_denthalpy_dp)[_qp] = dh_dp * _pressure_to_Pascals;
      (*_denthalpy_dT)[_qp] = dh_dT;
      if (_is_xnacl_pfvar)
        (*_denthalpy_dX)[_qp][0] = dh_dx;
    }
  }
}

template class PorousFlowBrineTempl<false>;
template class PorousFlowBrineTempl<true>;
