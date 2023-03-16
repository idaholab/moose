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

InputParameters
PorousFlowFluidStateSingleComponent::validParams()
{
  InputParameters params = PorousFlowVariableBase::validParams();
  params.addRequiredCoupledVar("porepressure",
                               "Variable that is the porepressure of the liquid phase");
  params.addRequiredCoupledVar("enthalpy", "Enthalpy of the fluid");
  MooseEnum unit_choice("Kelvin=0 Celsius=1", "Kelvin");
  params.addParam<MooseEnum>(
      "temperature_unit", unit_choice, "The unit of the temperature variable");
  params.addRequiredParam<UserObjectName>("capillary_pressure",
                                          "Name of the UserObject defining the capillary pressure");
  params.addRequiredParam<UserObjectName>("fluid_state", "Name of the FluidState UserObject");
  params.addPrivateParam<std::string>("pf_material_type", "fluid_state");
  params.addClassDescription(
      "Class for single component multiphase fluid state calculations using pressure and enthalpy");
  return params;
}

PorousFlowFluidStateSingleComponent::PorousFlowFluidStateSingleComponent(
    const InputParameters & parameters)
  : PorousFlowVariableBase(parameters),

    _liquid_porepressure(_nodal_material ? coupledDofValues("porepressure")
                                         : coupledValue("porepressure")),
    _liquid_gradp_qp(coupledGradient("porepressure")),
    _liquid_porepressure_varnum(coupled("porepressure")),
    _pvar(_dictator.isPorousFlowVariable(_liquid_porepressure_varnum)
              ? _dictator.porousFlowVariableNum(_liquid_porepressure_varnum)
              : 0),

    _enthalpy(_nodal_material ? coupledDofValues("enthalpy") : coupledValue("enthalpy")),
    _gradh_qp(coupledGradient("enthalpy")),
    _enthalpy_varnum(coupled("enthalpy")),
    _hvar(_dictator.isPorousFlowVariable(_enthalpy_varnum)
              ? _dictator.porousFlowVariableNum(_enthalpy_varnum)
              : 0),

    _fs(getUserObject<PorousFlowFluidStateSingleComponentBase>("fluid_state")),
    _aqueous_phase_number(_fs.aqueousPhaseIndex()),
    _gas_phase_number(_fs.gasPhaseIndex()),

    _temperature(_nodal_material ? declareProperty<Real>("PorousFlow_temperature_nodal")
                                 : declareProperty<Real>("PorousFlow_temperature_qp")),
    _grad_temperature_qp(_nodal_material
                             ? nullptr
                             : &declareProperty<RealGradient>("PorousFlow_grad_temperature_qp")),
    _dtemperature_dvar(
        _nodal_material ? declareProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")
                        : declareProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar")),
    _dgrad_temperature_dgradv(_nodal_material ? nullptr
                                              : &declareProperty<std::vector<Real>>(
                                                    "dPorousFlow_grad_temperature_qp_dgradvar")),
    _dgrad_temperature_dv(_nodal_material ? nullptr
                                          : &declareProperty<std::vector<RealGradient>>(
                                                "dPorousFlow_grad_temperature_qp_dvar")),
    _mass_frac(_nodal_material
                   ? declareProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")
                   : declareProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp")),
    _grad_mass_frac_qp(_nodal_material ? nullptr
                                       : &declareProperty<std::vector<std::vector<RealGradient>>>(
                                             "PorousFlow_grad_mass_frac_qp")),
    _dmass_frac_dvar(_nodal_material ? declareProperty<std::vector<std::vector<std::vector<Real>>>>(
                                           "dPorousFlow_mass_frac_nodal_dvar")
                                     : declareProperty<std::vector<std::vector<std::vector<Real>>>>(
                                           "dPorousFlow_mass_frac_qp_dvar")),

    _fluid_density(_nodal_material
                       ? declareProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_nodal")
                       : declareProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _dfluid_density_dvar(_nodal_material ? declareProperty<std::vector<std::vector<Real>>>(
                                               "dPorousFlow_fluid_phase_density_nodal_dvar")
                                         : declareProperty<std::vector<std::vector<Real>>>(
                                               "dPorousFlow_fluid_phase_density_qp_dvar")),
    _fluid_viscosity(_nodal_material
                         ? declareProperty<std::vector<Real>>("PorousFlow_viscosity_nodal")
                         : declareProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _dfluid_viscosity_dvar(
        _nodal_material
            ? declareProperty<std::vector<std::vector<Real>>>("dPorousFlow_viscosity_nodal_dvar")
            : declareProperty<std::vector<std::vector<Real>>>("dPorousFlow_viscosity_qp_dvar")),

    _fluid_enthalpy(
        _nodal_material
            ? declareProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_nodal")
            : declareProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_qp")),
    _dfluid_enthalpy_dvar(_nodal_material ? declareProperty<std::vector<std::vector<Real>>>(
                                                "dPorousFlow_fluid_phase_enthalpy_nodal_dvar")
                                          : declareProperty<std::vector<std::vector<Real>>>(
                                                "dPorousFlow_fluid_phase_enthalpy_qp_dvar")),

    _fluid_internal_energy(
        _nodal_material
            ? declareProperty<std::vector<Real>>("PorousFlow_fluid_phase_internal_energy_nodal")
            : declareProperty<std::vector<Real>>("PorousFlow_fluid_phase_internal_energy_qp")),
    _dfluid_internal_energy_dvar(_nodal_material
                                     ? declareProperty<std::vector<std::vector<Real>>>(
                                           "dPorousFlow_fluid_phase_internal_energy_nodal_dvar")
                                     : declareProperty<std::vector<std::vector<Real>>>(
                                           "dPorousFlow_fluid_phase_internal_qp_dvar")),

    _T_c2k(getParam<MooseEnum>("temperature_unit") == 0 ? 0.0 : 273.15),
    _is_initqp(false),
    _pc(getUserObject<PorousFlowCapillaryPressure>("capillary_pressure")),
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

  // Set the size of the FluidStateProperties vector
  _fsp.resize(_num_phases, FluidStateProperties(_num_components));
}

void
PorousFlowFluidStateSingleComponent::thermophysicalProperties()
{
  _fs.clearFluidStateProperties(_fsp);
  _fs.thermophysicalProperties(_liquid_porepressure[_qp], _enthalpy[_qp], _qp, _phase_state, _fsp);
}

void
PorousFlowFluidStateSingleComponent::initQpStatefulProperties()
{
  _is_initqp = true;
  // Set the size of pressure and saturation vectors
  PorousFlowVariableBase::initQpStatefulProperties();

  // Set the size of all other vectors
  setMaterialVectorSize();

  thermophysicalProperties();

  // Set the initial values of the properties at the nodes.
  // Note: not required for qp materials as no old values at the qps are requested
  if (_nodal_material)
  {
    thermophysicalProperties();

    // Temperature doesn't depend on fluid phase
    _temperature[_qp] = _fsp[_aqueous_phase_number].temperature.value() - _T_c2k;

    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      _saturation[_qp][ph] = _fsp[ph].saturation.value();
      _porepressure[_qp][ph] = _fsp[ph].pressure.value();
      _fluid_density[_qp][ph] = _fsp[ph].density.value();
      _fluid_viscosity[_qp][ph] = _fsp[ph].viscosity.value();
      _fluid_enthalpy[_qp][ph] = _fsp[ph].enthalpy.value();
      _fluid_internal_energy[_qp][ph] = _fsp[ph].internal_energy.value();
    }
  }
}

void
PorousFlowFluidStateSingleComponent::computeQpProperties()
{
  _is_initqp = false;
  // Prepare the derivative vectors
  PorousFlowVariableBase::computeQpProperties();

  // Set the size of all other vectors
  setMaterialVectorSize();

  // Calculate all required thermophysical properties
  thermophysicalProperties();

  // Temperature doesn't depend on fluid phase
  _temperature[_qp] = _fsp[_aqueous_phase_number].temperature.value() - _T_c2k;
  _dtemperature_dvar[_qp][_pvar] = _fsp[_aqueous_phase_number].temperature.derivatives()[_pidx];
  _dtemperature_dvar[_qp][_hvar] = _fsp[_aqueous_phase_number].temperature.derivatives()[_hidx];

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _saturation[_qp][ph] = _fsp[ph].saturation.value();
    _porepressure[_qp][ph] = _fsp[ph].pressure.value();
    _fluid_density[_qp][ph] = _fsp[ph].density.value();
    _fluid_viscosity[_qp][ph] = _fsp[ph].viscosity.value();
    _fluid_enthalpy[_qp][ph] = _fsp[ph].enthalpy.value();
    _fluid_internal_energy[_qp][ph] = _fsp[ph].internal_energy.value();
  }

  // Derivative of pressure, saturation and fluid properties wrt variables
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    (*_dporepressure_dvar)[_qp][ph][_pvar] = _fsp[ph].pressure.derivatives()[_pidx];
    (*_dporepressure_dvar)[_qp][ph][_hvar] = _fsp[ph].pressure.derivatives()[_hidx];

    (*_dsaturation_dvar)[_qp][ph][_pvar] = _fsp[ph].saturation.derivatives()[_pidx];
    (*_dsaturation_dvar)[_qp][ph][_hvar] = _fsp[ph].saturation.derivatives()[_hidx];

    _dfluid_density_dvar[_qp][ph][_pvar] = _fsp[ph].density.derivatives()[_pidx];
    _dfluid_density_dvar[_qp][ph][_hvar] = _fsp[ph].density.derivatives()[_hidx];

    _dfluid_viscosity_dvar[_qp][ph][_pvar] = _fsp[ph].viscosity.derivatives()[_pidx];
    _dfluid_viscosity_dvar[_qp][ph][_hvar] = _fsp[ph].viscosity.derivatives()[_hidx];

    _dfluid_enthalpy_dvar[_qp][ph][_pvar] = _fsp[ph].enthalpy.derivatives()[_pidx];
    _dfluid_enthalpy_dvar[_qp][ph][_hvar] = _fsp[ph].enthalpy.derivatives()[_hidx];

    _dfluid_internal_energy_dvar[_qp][ph][_pvar] = _fsp[ph].internal_energy.derivatives()[_pidx];
    _dfluid_internal_energy_dvar[_qp][ph][_hvar] = _fsp[ph].internal_energy.derivatives()[_hidx];
  }

  // If the material properties are being evaluated at the qps, calculate the
  // gradients as well. Note: only nodal properties are evaluated in
  // initQpStatefulProperties(), so no need to check _is_initqp flag for qp
  // properties
  if (!_nodal_material)
  {
    // Need to compute second derivatives of properties wrt variables for some of
    // the gradient derivatives. Use finite differences for now
    const Real dp = 1.0e-2;
    const Real dh = 1.0e-2;

    std::vector<FluidStateProperties> fsp_dp(_num_phases, FluidStateProperties(_num_components));
    _fs.thermophysicalProperties(
        _liquid_porepressure[_qp] + dp, _enthalpy[_qp], _qp, _phase_state, fsp_dp);

    std::vector<FluidStateProperties> fsp_dh(_num_phases, FluidStateProperties(_num_components));
    _fs.thermophysicalProperties(
        _liquid_porepressure[_qp], _enthalpy[_qp] + dh, _qp, _phase_state, fsp_dh);

    // Gradient of temperature (non-zero in all phases)
    (*_grad_temperature_qp)[_qp] = _dtemperature_dvar[_qp][_pvar] * _liquid_gradp_qp[_qp] +
                                   _dtemperature_dvar[_qp][_hvar] * _gradh_qp[_qp];
    (*_dgrad_temperature_dgradv)[_qp][_pvar] = _dtemperature_dvar[_qp][_pvar];
    (*_dgrad_temperature_dgradv)[_qp][_hvar] = _dtemperature_dvar[_qp][_hvar];

    const Real d2T_dp2 = (fsp_dp[_aqueous_phase_number].temperature.derivatives()[_pidx] -
                          _fsp[_aqueous_phase_number].temperature.derivatives()[_pidx]) /
                         dp;

    const Real d2T_dh2 = (fsp_dh[_aqueous_phase_number].temperature.derivatives()[_hidx] -
                          _fsp[_aqueous_phase_number].temperature.derivatives()[_hidx]) /
                         dh;

    const Real d2T_dph = (fsp_dp[_aqueous_phase_number].temperature.derivatives()[_hidx] -
                          _fsp[_aqueous_phase_number].temperature.derivatives()[_hidx]) /
                             (2.0 * dp) +
                         (fsp_dh[_aqueous_phase_number].temperature.derivatives()[_pidx] -
                          _fsp[_aqueous_phase_number].temperature.derivatives()[_pidx]) /
                             (2.0 * dh);

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
                             (2.0 * dp) +
                         (fsp_dh[_gas_phase_number].saturation.derivatives()[_pidx] -
                          _fsp[_gas_phase_number].saturation.derivatives()[_pidx]) /
                             (2.0 * dh);

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

void
PorousFlowFluidStateSingleComponent::setMaterialVectorSize() const
{
  _fluid_density[_qp].assign(_num_phases, 0.0);
  _fluid_viscosity[_qp].assign(_num_phases, 0.0);
  _fluid_enthalpy[_qp].assign(_num_phases, 0.0);
  _fluid_internal_energy[_qp].assign(_num_phases, 0.0);
  _mass_frac[_qp].resize(_num_phases);

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _mass_frac[_qp][ph].assign(_num_components, 1.0);

  // Derivatives and gradients are not required in initQpStatefulProperties
  if (!_is_initqp)
  {
    _dfluid_density_dvar[_qp].resize(_num_phases);
    _dfluid_viscosity_dvar[_qp].resize(_num_phases);
    _dfluid_enthalpy_dvar[_qp].resize(_num_phases);
    _dfluid_internal_energy_dvar[_qp].resize(_num_phases);
    _dmass_frac_dvar[_qp].resize(_num_phases);

    // Temperature doesn't depend of fluid phase
    _dtemperature_dvar[_qp].assign(_num_pf_vars, 0.0);

    if (!_nodal_material)
    {
      (*_grad_mass_frac_qp)[_qp].resize(_num_phases);
      (*_grad_temperature_qp)[_qp] = RealGradient();
      (*_dgrad_temperature_dgradv)[_qp].assign(_num_pf_vars, 0.0);
      (*_dgrad_temperature_dv)[_qp].assign(_num_pf_vars, RealGradient());
    }

    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      _dfluid_density_dvar[_qp][ph].assign(_num_pf_vars, 0.0);
      _dfluid_viscosity_dvar[_qp][ph].assign(_num_pf_vars, 0.0);
      _dfluid_enthalpy_dvar[_qp][ph].assign(_num_pf_vars, 0.0);
      _dfluid_internal_energy_dvar[_qp][ph].assign(_num_pf_vars, 0.0);
      _dmass_frac_dvar[_qp][ph].resize(_num_components);

      for (unsigned int comp = 0; comp < _num_components; ++comp)
        _dmass_frac_dvar[_qp][ph][comp].assign(_num_pf_vars, 0.0);

      if (!_nodal_material)
        (*_grad_mass_frac_qp)[_qp][ph].assign(_num_components, RealGradient());
    }
  }
}
