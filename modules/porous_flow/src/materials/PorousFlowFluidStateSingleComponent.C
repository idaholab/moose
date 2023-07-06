//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateSingleComponent.h"
#include "PorousFlowCapillaryPressure.h"

registerMooseObject("PorousFlowApp", PorousFlowFluidStateSingleComponent);
registerMooseObject("PorousFlowApp", ADPorousFlowFluidStateSingleComponent);

template <bool is_ad>
InputParameters
PorousFlowFluidStateSingleComponentTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowFluidStateBaseMaterialTempl<is_ad>::validParams();
  params.addRequiredCoupledVar("porepressure",
                               "Variable that is the porepressure of the liquid phase");
  params.addRequiredCoupledVar("enthalpy", "Enthalpy of the fluid");
  params.addRequiredParam<UserObjectName>("fluid_state", "Name of the FluidState UserObject");
  params.addClassDescription(
      "Class for single component multiphase fluid state calculations using pressure and enthalpy");
  return params;
}

template <bool is_ad>
PorousFlowFluidStateSingleComponentTempl<is_ad>::PorousFlowFluidStateSingleComponentTempl(
    const InputParameters & parameters)
  : PorousFlowFluidStateBaseMaterialTempl<is_ad>(parameters),
    _liquid_porepressure(_nodal_material
                             ? this->template coupledGenericDofValue<is_ad>("porepressure")
                             : this->template coupledGenericValue<is_ad>("porepressure")),
    _liquid_gradp_qp(this->template coupledGenericGradient<is_ad>("porepressure")),
    _liquid_porepressure_varnum(coupled("porepressure")),
    _pvar(_dictator.isPorousFlowVariable(_liquid_porepressure_varnum)
              ? _dictator.porousFlowVariableNum(_liquid_porepressure_varnum)
              : 0),
    _enthalpy(_nodal_material ? this->template coupledGenericDofValue<is_ad>("enthalpy")
                              : this->template coupledGenericValue<is_ad>("enthalpy")),
    _gradh_qp(this->template coupledGenericGradient<is_ad>("enthalpy")),
    _enthalpy_varnum(coupled("enthalpy")),
    _hvar(_dictator.isPorousFlowVariable(_enthalpy_varnum)
              ? _dictator.porousFlowVariableNum(_enthalpy_varnum)
              : 0),
    _fs(this->template getUserObject<PorousFlowFluidStateSingleComponentBase>("fluid_state")),
    _aqueous_phase_number(_fs.aqueousPhaseIndex()),
    _gas_phase_number(_fs.gasPhaseIndex()),
    _temperature(
        this->template declareGenericProperty<Real, is_ad>("PorousFlow_temperature" + _sfx)),
    _grad_temperature_qp(_nodal_material
                             ? nullptr
                             : &this->template declareGenericProperty<RealGradient, is_ad>(
                                   "PorousFlow_grad_temperature_qp")),
    _dtemperature_dvar(is_ad ? nullptr
                             : &this->template declareProperty<std::vector<Real>>(
                                   "dPorousFlow_temperature" + _sfx + "_dvar")),
    _dgrad_temperature_dgradv(is_ad || _nodal_material
                                  ? nullptr
                                  : &this->template declareProperty<std::vector<Real>>(
                                        "dPorousFlow_grad_temperature_qp_dgradvar")),
    _dgrad_temperature_dv(is_ad ? nullptr
                          : _nodal_material
                              ? nullptr
                              : &this->template declareProperty<std::vector<RealGradient>>(
                                    "dPorousFlow_grad_temperature_qp_dvar")),
    _pidx(_fs.getPressureIndex()),
    _hidx(_fs.getEnthalpyIndex())
{
  // Check that the number of phases in the fluidstate class is also provided in the Dictator
  if (_fs.numPhases() != _num_phases)
    mooseError(name(),
               ": only ",
               _fs.numPhases(),
               " phases are allowed. Please check the number of phases entered in the dictator is "
               "correct");
}

template <bool is_ad>
void
PorousFlowFluidStateSingleComponentTempl<is_ad>::thermophysicalProperties()
{
  _fs.clearFluidStateProperties(_fsp);
  _fs.thermophysicalProperties(_liquid_porepressure[_qp], _enthalpy[_qp], _qp, _fsp);
}

template <bool is_ad>
void
PorousFlowFluidStateSingleComponentTempl<is_ad>::initQpStatefulProperties()
{
  _is_initqp = true;
  // Set the size of pressure and saturation vectors
  PorousFlowFluidStateBaseMaterialTempl<is_ad>::initQpStatefulProperties();

  // Set the initial values of the temperature at the nodes.
  // Note: not required for qp materials as no old values at the qps are requested
  if (_nodal_material)
  {
    // Temperature doesn't depend on fluid phase
    _temperature[_qp] = genericValue(_fsp[_aqueous_phase_number].temperature) - _T_c2k;
  }
}

template <bool is_ad>
void
PorousFlowFluidStateSingleComponentTempl<is_ad>::computeQpProperties()
{

  PorousFlowFluidStateBaseMaterialTempl<is_ad>::computeQpProperties();

  // Temperature doesn't depend on fluid phase
  _temperature[_qp] = genericValue(_fsp[_aqueous_phase_number].temperature) - _T_c2k;

  if (!is_ad)
  {
    (*_dtemperature_dvar)[_qp][_pvar] =
        _fsp[_aqueous_phase_number].temperature.derivatives()[_pidx];
    (*_dtemperature_dvar)[_qp][_hvar] =
        _fsp[_aqueous_phase_number].temperature.derivatives()[_hidx];
  }

  // Derivative of pressure, saturation and fluid properties wrt variables
  if (!is_ad)
    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      (*_dporepressure_dvar)[_qp][ph][_pvar] = _fsp[ph].pressure.derivatives()[_pidx];
      (*_dporepressure_dvar)[_qp][ph][_hvar] = _fsp[ph].pressure.derivatives()[_hidx];

      (*_dsaturation_dvar)[_qp][ph][_pvar] = _fsp[ph].saturation.derivatives()[_pidx];
      (*_dsaturation_dvar)[_qp][ph][_hvar] = _fsp[ph].saturation.derivatives()[_hidx];

      (*_dfluid_density_dvar)[_qp][ph][_pvar] = _fsp[ph].density.derivatives()[_pidx];
      (*_dfluid_density_dvar)[_qp][ph][_hvar] = _fsp[ph].density.derivatives()[_hidx];

      (*_dfluid_viscosity_dvar)[_qp][ph][_pvar] = _fsp[ph].viscosity.derivatives()[_pidx];
      (*_dfluid_viscosity_dvar)[_qp][ph][_hvar] = _fsp[ph].viscosity.derivatives()[_hidx];

      (*_dfluid_enthalpy_dvar)[_qp][ph][_pvar] = _fsp[ph].enthalpy.derivatives()[_pidx];
      (*_dfluid_enthalpy_dvar)[_qp][ph][_hvar] = _fsp[ph].enthalpy.derivatives()[_hidx];

      (*_dfluid_internal_energy_dvar)[_qp][ph][_pvar] =
          _fsp[ph].internal_energy.derivatives()[_pidx];
      (*_dfluid_internal_energy_dvar)[_qp][ph][_hvar] =
          _fsp[ph].internal_energy.derivatives()[_hidx];
    }

  // If the material properties are being evaluated at the qps, calculate the
  // gradients as well. Note: only nodal properties are evaluated in
  // initQpStatefulProperties(), so no need to check _is_initqp flag for qp
  // properties
  if (!_nodal_material)
    if constexpr (!is_ad)
    {
      // Need to compute second derivatives of properties wrt variables for some of
      // the gradient derivatives. Use finite differences for now
      const Real dp = 1.0e-5 * _liquid_porepressure[_qp];
      const Real dh = 1.0e-5 * _enthalpy[_qp];

      std::vector<FluidStateProperties> fsp_dp(_num_phases, FluidStateProperties(_num_components));
      _fs.thermophysicalProperties(_liquid_porepressure[_qp] + dp, _enthalpy[_qp], _qp, fsp_dp);

      std::vector<FluidStateProperties> fsp_dh(_num_phases, FluidStateProperties(_num_components));
      _fs.thermophysicalProperties(_liquid_porepressure[_qp], _enthalpy[_qp] + dh, _qp, fsp_dh);

      // Gradient of temperature (non-zero in all phases)
      (*_grad_temperature_qp)[_qp] = (*_dtemperature_dvar)[_qp][_pvar] * _liquid_gradp_qp[_qp] +
                                     (*_dtemperature_dvar)[_qp][_hvar] * _gradh_qp[_qp];
      (*_dgrad_temperature_dgradv)[_qp][_pvar] = (*_dtemperature_dvar)[_qp][_pvar];
      (*_dgrad_temperature_dgradv)[_qp][_hvar] = (*_dtemperature_dvar)[_qp][_hvar];

      const auto d2T_dp2 = (fsp_dp[_aqueous_phase_number].temperature.derivatives()[_pidx] -
                            _fsp[_aqueous_phase_number].temperature.derivatives()[_pidx]) /
                           dp;

      const auto d2T_dh2 = (fsp_dh[_aqueous_phase_number].temperature.derivatives()[_hidx] -
                            _fsp[_aqueous_phase_number].temperature.derivatives()[_hidx]) /
                           dh;

      const auto d2T_dph = (fsp_dp[_aqueous_phase_number].temperature.derivatives()[_hidx] -
                            _fsp[_aqueous_phase_number].temperature.derivatives()[_hidx]) /
                               dp +
                           (fsp_dh[_aqueous_phase_number].temperature.derivatives()[_pidx] -
                            _fsp[_aqueous_phase_number].temperature.derivatives()[_pidx]) /
                               dh;

      (*_dgrad_temperature_dv)[_qp][_pvar] =
          d2T_dp2 * _liquid_gradp_qp[_qp] + d2T_dph * _gradh_qp[_qp];
      (*_dgrad_temperature_dv)[_qp][_hvar] =
          d2T_dph * _liquid_gradp_qp[_qp] + d2T_dh2 * _gradh_qp[_qp];

      // Gradient of saturation and derivatives
      (*_grads_qp)[_qp][_gas_phase_number] =
          (*_dsaturation_dvar)[_qp][_gas_phase_number][_pvar] * _liquid_gradp_qp[_qp] +
          (*_dsaturation_dvar)[_qp][_gas_phase_number][_hvar] * _gradh_qp[_qp];
      (*_grads_qp)[_qp][_aqueous_phase_number] = -(*_grads_qp)[_qp][_gas_phase_number];

      (*_dgrads_qp_dgradv)[_qp][_gas_phase_number][_pvar] =
          (*_dsaturation_dvar)[_qp][_gas_phase_number][_pvar];
      (*_dgrads_qp_dgradv)[_qp][_aqueous_phase_number][_pvar] =
          -(*_dgrads_qp_dgradv)[_qp][_gas_phase_number][_pvar];

      (*_dgrads_qp_dgradv)[_qp][_gas_phase_number][_hvar] =
          (*_dsaturation_dvar)[_qp][_gas_phase_number][_hvar];
      (*_dgrads_qp_dgradv)[_qp][_aqueous_phase_number][_hvar] =
          -(*_dgrads_qp_dgradv)[_qp][_gas_phase_number][_hvar];

      const Real d2s_dp2 = (fsp_dp[_gas_phase_number].saturation.derivatives()[_pidx] -
                            _fsp[_gas_phase_number].saturation.derivatives()[_pidx]) /
                           dp;

      const Real d2s_dh2 = (fsp_dh[_gas_phase_number].saturation.derivatives()[_hidx] -
                            _fsp[_gas_phase_number].saturation.derivatives()[_hidx]) /
                           dh;

      const Real d2s_dph = (fsp_dp[_gas_phase_number].saturation.derivatives()[_hidx] -
                            _fsp[_gas_phase_number].saturation.derivatives()[_hidx]) /
                               dp +
                           (fsp_dh[_gas_phase_number].saturation.derivatives()[_pidx] -
                            _fsp[_gas_phase_number].saturation.derivatives()[_pidx]) /
                               dh;

      (*_dgrads_qp_dv)[_qp][_gas_phase_number][_pvar] =
          d2s_dp2 * _liquid_gradp_qp[_qp] + d2s_dph * _gradh_qp[_qp];
      (*_dgrads_qp_dv)[_qp][_aqueous_phase_number][_pvar] =
          -(*_dgrads_qp_dv)[_qp][_gas_phase_number][_pvar];

      (*_dgrads_qp_dv)[_qp][_gas_phase_number][_hvar] =
          d2s_dh2 * _gradh_qp[_qp] + d2s_dph * _liquid_gradp_qp[_qp];
      (*_dgrads_qp_dv)[_qp][_aqueous_phase_number][_hvar] =
          -(*_dgrads_qp_dv)[_qp][_gas_phase_number][_hvar];

      // Gradient of porepressure and derivatives
      // Note: need first and second derivativea of capillary pressure
      const Real dpc = _pc.dCapillaryPressure(_fsp[_aqueous_phase_number].saturation.value());
      const Real d2pc = _pc.d2CapillaryPressure(_fsp[_aqueous_phase_number].saturation.value());

      (*_gradp_qp)[_qp][_aqueous_phase_number] = _liquid_gradp_qp[_qp];
      (*_gradp_qp)[_qp][_gas_phase_number] =
          _liquid_gradp_qp[_qp] + dpc * (*_grads_qp)[_qp][_aqueous_phase_number];

      for (unsigned int ph = 0; ph < _num_phases; ++ph)
        (*_dgradp_qp_dgradv)[_qp][ph][_pvar] = 1.0;

      (*_dgradp_qp_dgradv)[_qp][_gas_phase_number][_pvar] +=
          dpc * (*_dgrads_qp_dgradv)[_qp][_aqueous_phase_number][_pvar];
      (*_dgradp_qp_dgradv)[_qp][_gas_phase_number][_hvar] =
          dpc * (*_dgrads_qp_dgradv)[_qp][_aqueous_phase_number][_hvar];

      (*_dgradp_qp_dv)[_qp][_gas_phase_number][_pvar] =
          d2pc * (*_grads_qp)[_qp][_aqueous_phase_number] *
              (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_pvar] +
          dpc * (*_dgrads_qp_dv)[_qp][_aqueous_phase_number][_pvar];

      (*_dgradp_qp_dv)[_qp][_gas_phase_number][_hvar] =
          d2pc * (*_grads_qp)[_qp][_aqueous_phase_number] *
              (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_hvar] +
          dpc * (*_dgrads_qp_dv)[_qp][_aqueous_phase_number][_hvar];
    }
}

template <bool is_ad>
void
PorousFlowFluidStateSingleComponentTempl<is_ad>::setMaterialVectorSize() const
{
  PorousFlowFluidStateBaseMaterialTempl<is_ad>::setMaterialVectorSize();

  // Derivatives and gradients are not required in initQpStatefulProperties
  if (!_is_initqp)
  {
    // Temperature doesn't depend of fluid phase
    (*_dtemperature_dvar)[_qp].assign(_num_pf_vars, 0.0);

    // The gradient of the temperature is needed for qp materials and AD materials
    if (!_nodal_material || is_ad)
      (*_grad_temperature_qp)[_qp] = RealGradient();

    // No derivatives are required for AD materials
    if (!is_ad)
      if (!_nodal_material)
      {
        (*_dgrad_temperature_dgradv)[_qp].assign(_num_pf_vars, 0.0);
        (*_dgrad_temperature_dv)[_qp].assign(_num_pf_vars, RealGradient());
      }
  }
}

template class PorousFlowFluidStateSingleComponentTempl<false>;
template class PorousFlowFluidStateSingleComponentTempl<true>;
