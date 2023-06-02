//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidState.h"
#include "PorousFlowCapillaryPressure.h"

registerMooseObject("PorousFlowApp", PorousFlowFluidState);

InputParameters
PorousFlowFluidState::validParams()
{
  InputParameters params = PorousFlowVariableBase::validParams();
  params.addRequiredCoupledVar("gas_porepressure",
                               "Variable that is the porepressure of the gas phase");
  params.addRequiredCoupledVar("z", "Total mass fraction of component i summed over all phases");
  params.addCoupledVar(
      "temperature", 20, "The fluid temperature (C or K, depending on temperature_unit)");
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  MooseEnum unit_choice("Kelvin=0 Celsius=1", "Kelvin");
  params.addParam<MooseEnum>(
      "temperature_unit", unit_choice, "The unit of the temperature variable");
  params.addRequiredParam<UserObjectName>("capillary_pressure",
                                          "Name of the UserObject defining the capillary pressure");
  params.addRequiredParam<UserObjectName>("fluid_state", "Name of the FluidState UserObject");
  params.addPrivateParam<std::string>("pf_material_type", "fluid_state");
  params.addClassDescription("Class for fluid state calculations using persistent primary "
                             "variables and a vapor-liquid flash");
  return params;
}

PorousFlowFluidState::PorousFlowFluidState(const InputParameters & parameters)
  : PorousFlowVariableBase(parameters),

    _gas_porepressure(_nodal_material ? coupledDofValues("gas_porepressure")
                                      : coupledValue("gas_porepressure")),
    _gas_gradp_qp(coupledGradient("gas_porepressure")),
    _gas_porepressure_varnum(coupled("gas_porepressure")),
    _pvar(_dictator.isPorousFlowVariable(_gas_porepressure_varnum)
              ? _dictator.porousFlowVariableNum(_gas_porepressure_varnum)
              : 0),

    _num_Z_vars(coupledComponents("z")),
    _is_Xnacl_nodal(isCoupled("xnacl") ? getVar("xnacl", 0)->isNodal() : false),
    _Xnacl(_nodal_material && _is_Xnacl_nodal ? coupledDofValues("xnacl") : coupledValue("xnacl")),
    _grad_Xnacl_qp(coupledGradient("xnacl")),
    _Xnacl_varnum(coupled("xnacl")),
    _Xvar(_dictator.isPorousFlowVariable(_Xnacl_varnum)
              ? _dictator.porousFlowVariableNum(_Xnacl_varnum)
              : 0),

    _fs(getUserObject<PorousFlowFluidStateMultiComponentBase>("fluid_state")),
    _aqueous_phase_number(_fs.aqueousPhaseIndex()),
    _gas_phase_number(_fs.gasPhaseIndex()),
    _aqueous_fluid_component(_fs.aqueousComponentIndex()),
    _gas_fluid_component(_fs.gasComponentIndex()),
    _salt_component(_fs.saltComponentIndex()),

    _temperature(_nodal_material ? getMaterialProperty<Real>("PorousFlow_temperature_nodal")
                                 : getMaterialProperty<Real>("PorousFlow_temperature_qp")),
    _gradT_qp(getMaterialProperty<RealGradient>("PorousFlow_grad_temperature_qp")),
    _dtemperature_dvar(
        _nodal_material
            ? getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")
            : getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar")),
    _temperature_varnum(coupled("temperature")),
    _Tvar(_dictator.isPorousFlowVariable(_temperature_varnum)
              ? _dictator.porousFlowVariableNum(_temperature_varnum)
              : 0),
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
                                           "dPorousFlow_fluid_phase_internal_energy_qp_dvar")),

    _T_c2k(getParam<MooseEnum>("temperature_unit") == 0 ? 0.0 : 273.15),
    _is_initqp(false),
    _pc(getUserObject<PorousFlowCapillaryPressure>("capillary_pressure")),
    _pidx(_fs.getPressureIndex()),
    _Tidx(_fs.getTemperatureIndex()),
    _Zidx(_fs.getZIndex()),
    _Xidx(_fs.getXIndex())
{
  // Check that the number of phases in the fluidstate class is also provided in the Dictator
  if (_fs.numPhases() != _num_phases)
    mooseError(name(),
               ": only ",
               _fs.numPhases(),
               " phases are allowed. Please check the number of phases entered in the dictator is "
               "correct");

  // Store all total mass fractions and associated variable numbers
  _Z.resize(_num_Z_vars);
  _gradZ_qp.resize(_num_Z_vars);
  _Z_varnum.resize(_num_Z_vars);
  _Zvar.resize(_num_Z_vars);

  for (unsigned int i = 0; i < _num_Z_vars; ++i)
  {
    _Z[i] = (_nodal_material ? &coupledDofValues("z", i) : &coupledValue("z", i));
    _gradZ_qp[i] = &coupledGradient("z", i);
    _Z_varnum[i] = coupled("z", i);
    _Zvar[i] = (_dictator.isPorousFlowVariable(_Z_varnum[i])
                    ? _dictator.porousFlowVariableNum(_Z_varnum[i])
                    : 0);
  }

  // Set the size of the FluidStateProperties vector
  _fsp.resize(_num_phases, FluidStateProperties(_num_components));
}

void
PorousFlowFluidState::thermophysicalProperties()
{
  // The FluidProperty objects use temperature in K
  Real Tk = _temperature[_qp] + _T_c2k;

  _fs.thermophysicalProperties(_gas_porepressure[_qp], Tk, _Xnacl[_qp], (*_Z[0])[_qp], _qp, _fsp);
}

void
PorousFlowFluidState::initQpStatefulProperties()
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
    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      _saturation[_qp][ph] = _fsp[ph].saturation.value();
      _porepressure[_qp][ph] = _fsp[ph].pressure.value();
      _fluid_density[_qp][ph] = _fsp[ph].density.value();
      _fluid_viscosity[_qp][ph] = _fsp[ph].viscosity.value();
      _fluid_enthalpy[_qp][ph] = _fsp[ph].enthalpy.value();
      _fluid_internal_energy[_qp][ph] = _fsp[ph].internal_energy.value();

      for (unsigned int comp = 0; comp < _num_components; ++comp)
        _mass_frac[_qp][ph][comp] = _fsp[ph].mass_fraction[comp].value();
    }
}

void
PorousFlowFluidState::computeQpProperties()
{
  _is_initqp = false;
  // Prepare the derivative vectors
  PorousFlowVariableBase::computeQpProperties();

  // Set the size of all other vectors
  setMaterialVectorSize();

  // Calculate all required thermophysical properties
  thermophysicalProperties();

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _saturation[_qp][ph] = _fsp[ph].saturation.value();
    _porepressure[_qp][ph] = _fsp[ph].pressure.value();
    _fluid_density[_qp][ph] = _fsp[ph].density.value();
    _fluid_viscosity[_qp][ph] = _fsp[ph].viscosity.value();
    _fluid_enthalpy[_qp][ph] = _fsp[ph].enthalpy.value();
    _fluid_internal_energy[_qp][ph] = _fsp[ph].internal_energy.value();

    for (unsigned int comp = 0; comp < _num_components; ++comp)
      _mass_frac[_qp][ph][comp] = _fsp[ph].mass_fraction[comp].value();
  }

  // Derivative of properties wrt variables (calculated in fluid state class)
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    // If porepressure is a PorousFlow variable (it usually is), add derivatives wrt porepressure
    if (_dictator.isPorousFlowVariable(_gas_porepressure_varnum))
    {
      (*_dporepressure_dvar)[_qp][ph][_pvar] = _fsp[ph].pressure.derivatives()[_pidx];
      (*_dsaturation_dvar)[_qp][ph][_pvar] = _fsp[ph].saturation.derivatives()[_pidx];
      _dfluid_density_dvar[_qp][ph][_pvar] = _fsp[ph].density.derivatives()[_pidx];
      _dfluid_viscosity_dvar[_qp][ph][_pvar] = _fsp[ph].viscosity.derivatives()[_pidx];
      _dfluid_enthalpy_dvar[_qp][ph][_pvar] = _fsp[ph].enthalpy.derivatives()[_pidx];
      _dfluid_internal_energy_dvar[_qp][ph][_pvar] = _fsp[ph].internal_energy.derivatives()[_pidx];

      for (unsigned int comp = 0; comp < _num_components; ++comp)
        _dmass_frac_dvar[_qp][ph][comp][_pvar] = _fsp[ph].mass_fraction[comp].derivatives()[_pidx];
    }

    // If Z is a PorousFlow variable (it usually is), add derivatives wrt Z
    if (_dictator.isPorousFlowVariable(_Z_varnum[0]))
    {
      (*_dporepressure_dvar)[_qp][ph][_Zvar[0]] = _fsp[ph].pressure.derivatives()[_Zidx];
      (*_dsaturation_dvar)[_qp][ph][_Zvar[0]] = _fsp[ph].saturation.derivatives()[_Zidx];
      _dfluid_density_dvar[_qp][ph][_Zvar[0]] = _fsp[ph].density.derivatives()[_Zidx];
      _dfluid_viscosity_dvar[_qp][ph][_Zvar[0]] = _fsp[ph].viscosity.derivatives()[_Zidx];
      _dfluid_enthalpy_dvar[_qp][ph][_Zvar[0]] = _fsp[ph].enthalpy.derivatives()[_Zidx];
      _dfluid_internal_energy_dvar[_qp][ph][_Zvar[0]] =
          _fsp[ph].internal_energy.derivatives()[_Zidx];

      for (unsigned int comp = 0; comp < _num_components; ++comp)
        _dmass_frac_dvar[_qp][ph][comp][_Zvar[0]] =
            _fsp[ph].mass_fraction[comp].derivatives()[_Zidx];
    }

    // If temperature is a PorousFlow variable (nonisothermal case), add derivatives wrt temperature
    if (_dictator.isPorousFlowVariable(_temperature_varnum))
    {
      (*_dporepressure_dvar)[_qp][ph][_Tvar] = _fsp[ph].pressure.derivatives()[_Tidx];
      (*_dsaturation_dvar)[_qp][ph][_Tvar] = _fsp[ph].saturation.derivatives()[_Tidx];
      _dfluid_density_dvar[_qp][ph][_Tvar] = _fsp[ph].density.derivatives()[_Tidx];
      _dfluid_viscosity_dvar[_qp][ph][_Tvar] = _fsp[ph].viscosity.derivatives()[_Tidx];
      _dfluid_enthalpy_dvar[_qp][ph][_Tvar] = _fsp[ph].enthalpy.derivatives()[_Tidx];
      _dfluid_internal_energy_dvar[_qp][ph][_Tvar] = _fsp[ph].internal_energy.derivatives()[_Tidx];

      for (unsigned int comp = 0; comp < _num_components; ++comp)
        _dmass_frac_dvar[_qp][ph][comp][_Tvar] = _fsp[ph].mass_fraction[comp].derivatives()[_Tidx];
    }

    // If Xnacl is a PorousFlow variable, add derivatives wrt Xnacl
    if (_dictator.isPorousFlowVariable(_Xnacl_varnum))
    {
      (*_dporepressure_dvar)[_qp][ph][_Xvar] = _fsp[ph].pressure.derivatives()[_Xidx];
      (*_dsaturation_dvar)[_qp][ph][_Xvar] = _fsp[ph].saturation.derivatives()[_Xidx];
      _dfluid_density_dvar[_qp][ph][_Xvar] += _fsp[ph].density.derivatives()[_Xidx];
      _dfluid_viscosity_dvar[_qp][ph][_Xvar] += _fsp[ph].viscosity.derivatives()[_Xidx];
      _dfluid_enthalpy_dvar[_qp][ph][_Xvar] = _fsp[ph].enthalpy.derivatives()[_Xidx];
      _dfluid_internal_energy_dvar[_qp][ph][_Xvar] = _fsp[ph].internal_energy.derivatives()[_Xidx];

      for (unsigned int comp = 0; comp < _num_components; ++comp)
        _dmass_frac_dvar[_qp][ph][comp][_Xvar] = _fsp[ph].mass_fraction[comp].derivatives()[_Xidx];
    }
  }

  // If the material properties are being evaluated at the qps, calculate the gradients as well
  // Note: only nodal properties are evaluated in initQpStatefulProperties(), so no need to check
  // _is_initqp flag for qp properties
  if (!_nodal_material)
  {
    // Derivatives of capillary pressure
    const Real dpc = _pc.dCapillaryPressure(_fsp[_aqueous_phase_number].saturation.value(), _qp);
    const Real d2pc = _pc.d2CapillaryPressure(_fsp[_aqueous_phase_number].saturation.value(), _qp);

    // Gradients of saturation and porepressure in all phases
    (*_grads_qp)[_qp][_gas_phase_number] =
        (*_dsaturation_dvar)[_qp][_gas_phase_number][_pvar] * _gas_gradp_qp[_qp] +
        (*_dsaturation_dvar)[_qp][_gas_phase_number][_Zvar[0]] * (*_gradZ_qp[0])[_qp] +
        (*_dsaturation_dvar)[_qp][_gas_phase_number][_Tvar] * _gradT_qp[_qp];
    (*_grads_qp)[_qp][_aqueous_phase_number] = -(*_grads_qp)[_qp][_gas_phase_number];

    (*_gradp_qp)[_qp][_gas_phase_number] = _gas_gradp_qp[_qp];
    (*_gradp_qp)[_qp][_aqueous_phase_number] =
        _gas_gradp_qp[_qp] - dpc * (*_grads_qp)[_qp][_aqueous_phase_number];

    // Gradients of mass fractions for each component in each phase
    (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_aqueous_fluid_component] =
        _fsp[_aqueous_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_pidx] *
            _gas_gradp_qp[_qp] +
        _fsp[_aqueous_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_Zidx] *
            (*_gradZ_qp[0])[_qp] +
        _fsp[_aqueous_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_Tidx] *
            _gradT_qp[_qp];
    (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_gas_fluid_component] =
        -(*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_aqueous_fluid_component];

    (*_grad_mass_frac_qp)[_qp][_gas_phase_number][_aqueous_fluid_component] =
        _fsp[_gas_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_pidx] *
            _gas_gradp_qp[_qp] +
        _fsp[_gas_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_Zidx] *
            (*_gradZ_qp[0])[_qp] +
        _fsp[_gas_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_Tidx] *
            _gradT_qp[_qp];
    (*_grad_mass_frac_qp)[_qp][_gas_phase_number][_gas_fluid_component] =
        -(*_grad_mass_frac_qp)[_qp][_gas_phase_number][_aqueous_fluid_component];

    // Derivatives of gradients wrt variables
    if (_dictator.isPorousFlowVariable(_gas_porepressure_varnum))
    {
      for (unsigned int ph = 0; ph < _num_phases; ++ph)
        (*_dgradp_qp_dgradv)[_qp][ph][_pvar] = 1.0;

      (*_dgradp_qp_dgradv)[_qp][_aqueous_phase_number][_pvar] +=
          -dpc * (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_pvar];

      (*_dgradp_qp_dv)[_qp][_aqueous_phase_number][_pvar] =
          -d2pc * (*_grads_qp)[_qp][_aqueous_phase_number] *
          (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_pvar];
    }

    if (_dictator.isPorousFlowVariable(_Z_varnum[0]))
    {
      (*_dgradp_qp_dgradv)[_qp][_aqueous_phase_number][_Zvar[0]] =
          -dpc * (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_Zvar[0]];

      (*_dgradp_qp_dv)[_qp][_aqueous_phase_number][_Zvar[0]] =
          -d2pc * (*_grads_qp)[_qp][_aqueous_phase_number] *
          (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_Zvar[0]];
    }

    if (_dictator.isPorousFlowVariable(_temperature_varnum))
    {
      (*_dgradp_qp_dgradv)[_qp][_aqueous_phase_number][_Tvar] =
          -dpc * (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_Tvar];

      (*_dgradp_qp_dv)[_qp][_aqueous_phase_number][_Tvar] =
          -d2pc * (*_grads_qp)[_qp][_aqueous_phase_number] *
          (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_Tvar];
    }

    // If Xnacl is a PorousFlow variable, add gradients and derivatives wrt Xnacl
    if (_dictator.isPorousFlowVariable(_Xnacl_varnum))
    {
      (*_dgradp_qp_dgradv)[_qp][_aqueous_phase_number][_Xvar] =
          -dpc * (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_Xvar];

      (*_grads_qp)[_qp][_aqueous_phase_number] +=
          (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_Xvar] * _grad_Xnacl_qp[_qp];

      (*_grads_qp)[_qp][_gas_phase_number] -=
          (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_Xvar] * _grad_Xnacl_qp[_qp];

      (*_gradp_qp)[_qp][_aqueous_phase_number] -=
          dpc * (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_Xvar] * _grad_Xnacl_qp[_qp];

      (*_dgradp_qp_dv)[_qp][_aqueous_phase_number][_Xvar] =
          -d2pc * (*_grads_qp)[_qp][_aqueous_phase_number] *
          (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_Xvar];

      (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_salt_component] = _grad_Xnacl_qp[_qp];
      (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_aqueous_fluid_component] +=
          _fsp[_aqueous_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_Xidx] *
          _grad_Xnacl_qp[_qp];
      (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_gas_fluid_component] -=
          _fsp[_aqueous_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_Xidx] *
          _grad_Xnacl_qp[_qp];
      (*_grad_mass_frac_qp)[_qp][_gas_phase_number][_aqueous_fluid_component] +=
          _fsp[_gas_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_Xidx] *
          _grad_Xnacl_qp[_qp];
      (*_grad_mass_frac_qp)[_qp][_gas_phase_number][_gas_fluid_component] -=
          _fsp[_gas_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_Xidx] *
          _grad_Xnacl_qp[_qp];
    }
  }
}

void
PorousFlowFluidState::setMaterialVectorSize() const
{
  _fluid_density[_qp].assign(_num_phases, 0.0);
  _fluid_viscosity[_qp].assign(_num_phases, 0.0);
  _fluid_enthalpy[_qp].assign(_num_phases, 0.0);
  _fluid_internal_energy[_qp].assign(_num_phases, 0.0);
  _mass_frac[_qp].resize(_num_phases);

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _mass_frac[_qp][ph].resize(_num_components);

  // Derivatives and gradients are not required in initQpStatefulProperties
  if (!_is_initqp)
  {
    _dfluid_density_dvar[_qp].resize(_num_phases);
    _dfluid_viscosity_dvar[_qp].resize(_num_phases);
    _dfluid_enthalpy_dvar[_qp].resize(_num_phases);
    _dfluid_internal_energy_dvar[_qp].resize(_num_phases);
    _dmass_frac_dvar[_qp].resize(_num_phases);

    if (!_nodal_material)
      (*_grad_mass_frac_qp)[_qp].resize(_num_phases);

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
