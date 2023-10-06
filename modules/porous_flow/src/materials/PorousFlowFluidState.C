//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidState.h"

registerMooseObject("PorousFlowApp", PorousFlowFluidState);
registerMooseObject("PorousFlowApp", ADPorousFlowFluidState);

template <bool is_ad>
InputParameters
PorousFlowFluidStateTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowFluidStateBaseMaterialTempl<is_ad>::validParams();
  params.addRequiredCoupledVar("gas_porepressure",
                               "Variable that is the porepressure of the gas phase");
  params.addRequiredCoupledVar("z", "Total mass fraction of component i summed over all phases");
  params.addCoupledVar(
      "temperature", 20, "The fluid temperature (C or K, depending on temperature_unit)");
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  params.addRequiredParam<UserObjectName>("fluid_state", "Name of the FluidState UserObject");
  params.addClassDescription("Class for fluid state calculations using persistent primary "
                             "variables and a vapor-liquid flash");
  return params;
}

template <bool is_ad>
PorousFlowFluidStateTempl<is_ad>::PorousFlowFluidStateTempl(const InputParameters & parameters)
  : PorousFlowFluidStateBaseMaterialTempl<is_ad>(parameters),
    _gas_porepressure(_nodal_material
                          ? this->template coupledGenericDofValue<is_ad>("gas_porepressure")
                          : this->template coupledGenericValue<is_ad>("gas_porepressure")),
    _gas_gradp_qp(this->template coupledGenericGradient<is_ad>("gas_porepressure")),
    _gas_porepressure_varnum(coupled("gas_porepressure")),
    _pvar(_dictator.isPorousFlowVariable(_gas_porepressure_varnum)
              ? _dictator.porousFlowVariableNum(_gas_porepressure_varnum)
              : 0),
    _num_Z_vars(coupledComponents("z")),
    _is_Xnacl_nodal(isCoupled("xnacl") ? getFieldVar("xnacl", 0)->isNodal() : false),
    _Xnacl(_nodal_material && _is_Xnacl_nodal
               ? this->template coupledGenericDofValue<is_ad>("xnacl")
               : this->template coupledGenericValue<is_ad>("xnacl")),
    _grad_Xnacl_qp(this->template coupledGenericGradient<is_ad>("xnacl")),
    _Xnacl_varnum(coupled("xnacl")),
    _Xvar(_dictator.isPorousFlowVariable(_Xnacl_varnum)
              ? _dictator.porousFlowVariableNum(_Xnacl_varnum)
              : 0),
    _fs(this->template getUserObject<PorousFlowFluidStateMultiComponentBase>("fluid_state")),
    _aqueous_phase_number(_fs.aqueousPhaseIndex()),
    _gas_phase_number(_fs.gasPhaseIndex()),
    _aqueous_fluid_component(_fs.aqueousComponentIndex()),
    _gas_fluid_component(_fs.gasComponentIndex()),
    _salt_component(_fs.saltComponentIndex()),
    _temperature(
        this->template getGenericMaterialProperty<Real, is_ad>("PorousFlow_temperature" + _sfx)),
    _gradT_qp(_nodal_material ? nullptr
                              : &this->template getGenericMaterialProperty<RealGradient, is_ad>(
                                    "PorousFlow_grad_temperature" + _sfx)),
    _dtemperature_dvar(is_ad ? nullptr
                             : &this->template getMaterialProperty<std::vector<Real>>(
                                   "dPorousFlow_temperature" + _sfx + "_dvar")),
    _temperature_varnum(coupled("temperature")),
    _Tvar(_dictator.isPorousFlowVariable(_temperature_varnum)
              ? _dictator.porousFlowVariableNum(_temperature_varnum)
              : 0),
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
    _Z[i] = (_nodal_material ? &this->template coupledGenericDofValue<is_ad>("z", i)
                             : &this->template coupledGenericValue<is_ad>("z", i));
    _gradZ_qp[i] = &this->template coupledGenericGradient<is_ad>("z", i);
    _Z_varnum[i] = coupled("z", i);
    _Zvar[i] = (_dictator.isPorousFlowVariable(_Z_varnum[i])
                    ? _dictator.porousFlowVariableNum(_Z_varnum[i])
                    : 0);
  }
}

template <bool is_ad>
void
PorousFlowFluidStateTempl<is_ad>::thermophysicalProperties()
{
  // The FluidProperty objects use temperature in K
  const GenericReal<is_ad> Tk = _temperature[_qp] + _T_c2k;

  _fs.clearFluidStateProperties(_fsp);
  _fs.thermophysicalProperties(_gas_porepressure[_qp], Tk, _Xnacl[_qp], (*_Z[0])[_qp], _qp, _fsp);
}

template <bool is_ad>
void
PorousFlowFluidStateTempl<is_ad>::initQpStatefulProperties()
{
  PorousFlowFluidStateBaseMaterialTempl<is_ad>::initQpStatefulProperties();
}

template <bool is_ad>
void
PorousFlowFluidStateTempl<is_ad>::computeQpProperties()
{
  PorousFlowFluidStateBaseMaterialTempl<is_ad>::computeQpProperties();

  // If the material isn't AD, we need to compute the derivatives
  if (!is_ad)
  {
    // Derivative of properties wrt variables (calculated in fluid state class)
    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      // If porepressure is a PorousFlow variable (it usually is), add derivatives wrt
      // porepressure
      if (_dictator.isPorousFlowVariable(_gas_porepressure_varnum))
      {
        (*_dporepressure_dvar)[_qp][ph][_pvar] = _fsp[ph].pressure.derivatives()[_pidx];
        (*_dsaturation_dvar)[_qp][ph][_pvar] = _fsp[ph].saturation.derivatives()[_pidx];
        (*_dfluid_density_dvar)[_qp][ph][_pvar] = _fsp[ph].density.derivatives()[_pidx];
        (*_dfluid_viscosity_dvar)[_qp][ph][_pvar] = _fsp[ph].viscosity.derivatives()[_pidx];
        (*_dfluid_enthalpy_dvar)[_qp][ph][_pvar] = _fsp[ph].enthalpy.derivatives()[_pidx];
        (*_dfluid_internal_energy_dvar)[_qp][ph][_pvar] =
            _fsp[ph].internal_energy.derivatives()[_pidx];

        for (unsigned int comp = 0; comp < _num_components; ++comp)
          (*_dmass_frac_dvar)[_qp][ph][comp][_pvar] =
              _fsp[ph].mass_fraction[comp].derivatives()[_pidx];
      }

      // If Z is a PorousFlow variable (it usually is), add derivatives wrt Z
      if (_dictator.isPorousFlowVariable(_Z_varnum[0]))
      {
        (*_dporepressure_dvar)[_qp][ph][_Zvar[0]] = _fsp[ph].pressure.derivatives()[_Zidx];
        (*_dsaturation_dvar)[_qp][ph][_Zvar[0]] = _fsp[ph].saturation.derivatives()[_Zidx];
        (*_dfluid_density_dvar)[_qp][ph][_Zvar[0]] = _fsp[ph].density.derivatives()[_Zidx];
        (*_dfluid_viscosity_dvar)[_qp][ph][_Zvar[0]] = _fsp[ph].viscosity.derivatives()[_Zidx];
        (*_dfluid_enthalpy_dvar)[_qp][ph][_Zvar[0]] = _fsp[ph].enthalpy.derivatives()[_Zidx];
        (*_dfluid_internal_energy_dvar)[_qp][ph][_Zvar[0]] =
            _fsp[ph].internal_energy.derivatives()[_Zidx];

        for (unsigned int comp = 0; comp < _num_components; ++comp)
          (*_dmass_frac_dvar)[_qp][ph][comp][_Zvar[0]] =
              _fsp[ph].mass_fraction[comp].derivatives()[_Zidx];
      }

      // If temperature is a PorousFlow variable (nonisothermal case), add derivatives wrt
      // temperature
      if (_dictator.isPorousFlowVariable(_temperature_varnum))
      {
        (*_dporepressure_dvar)[_qp][ph][_Tvar] = _fsp[ph].pressure.derivatives()[_Tidx];
        (*_dsaturation_dvar)[_qp][ph][_Tvar] = _fsp[ph].saturation.derivatives()[_Tidx];
        (*_dfluid_density_dvar)[_qp][ph][_Tvar] = _fsp[ph].density.derivatives()[_Tidx];
        (*_dfluid_viscosity_dvar)[_qp][ph][_Tvar] = _fsp[ph].viscosity.derivatives()[_Tidx];
        (*_dfluid_enthalpy_dvar)[_qp][ph][_Tvar] = _fsp[ph].enthalpy.derivatives()[_Tidx];
        (*_dfluid_internal_energy_dvar)[_qp][ph][_Tvar] =
            _fsp[ph].internal_energy.derivatives()[_Tidx];

        for (unsigned int comp = 0; comp < _num_components; ++comp)
          (*_dmass_frac_dvar)[_qp][ph][comp][_Tvar] =
              _fsp[ph].mass_fraction[comp].derivatives()[_Tidx];
      }

      // If Xnacl is a PorousFlow variable, add derivatives wrt Xnacl
      if (_dictator.isPorousFlowVariable(_Xnacl_varnum))
      {
        (*_dporepressure_dvar)[_qp][ph][_Xvar] = _fsp[ph].pressure.derivatives()[_Xidx];
        (*_dsaturation_dvar)[_qp][ph][_Xvar] = _fsp[ph].saturation.derivatives()[_Xidx];
        (*_dfluid_density_dvar)[_qp][ph][_Xvar] += _fsp[ph].density.derivatives()[_Xidx];
        (*_dfluid_viscosity_dvar)[_qp][ph][_Xvar] += _fsp[ph].viscosity.derivatives()[_Xidx];
        (*_dfluid_enthalpy_dvar)[_qp][ph][_Xvar] = _fsp[ph].enthalpy.derivatives()[_Xidx];
        (*_dfluid_internal_energy_dvar)[_qp][ph][_Xvar] =
            _fsp[ph].internal_energy.derivatives()[_Xidx];

        for (unsigned int comp = 0; comp < _num_components; ++comp)
          (*_dmass_frac_dvar)[_qp][ph][comp][_Xvar] =
              _fsp[ph].mass_fraction[comp].derivatives()[_Xidx];
      }
    }
  }

  // If the material properties are being evaluated at the qps, calculate the gradients as well
  // Note: only nodal properties are evaluated in initQpStatefulProperties(), so no need to check
  // _is_initqp flag for qp properties
  if (!_nodal_material)
    if constexpr (!is_ad)
    {
      // Derivatives of capillary pressure
      const Real dpc = _pc.dCapillaryPressure(_fsp[_aqueous_phase_number].saturation.value(), _qp);
      const Real d2pc =
          _pc.d2CapillaryPressure(_fsp[_aqueous_phase_number].saturation.value(), _qp);

      // Gradients of saturation and porepressure in all phases
      (*_grads_qp)[_qp][_gas_phase_number] =
          (*_dsaturation_dvar)[_qp][_gas_phase_number][_pvar] * _gas_gradp_qp[_qp] +
          (*_dsaturation_dvar)[_qp][_gas_phase_number][_Zvar[0]] * (*_gradZ_qp[0])[_qp] +
          (*_dsaturation_dvar)[_qp][_gas_phase_number][_Tvar] * (*_gradT_qp)[_qp];
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
              (*_gradT_qp)[_qp];
      (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_gas_fluid_component] =
          -(*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_aqueous_fluid_component];

      (*_grad_mass_frac_qp)[_qp][_gas_phase_number][_aqueous_fluid_component] =
          _fsp[_gas_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_pidx] *
              _gas_gradp_qp[_qp] +
          _fsp[_gas_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_Zidx] *
              (*_gradZ_qp[0])[_qp] +
          _fsp[_gas_phase_number].mass_fraction[_aqueous_fluid_component].derivatives()[_Tidx] *
              (*_gradT_qp)[_qp];
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
            _fsp[_aqueous_phase_number]
                .mass_fraction[_aqueous_fluid_component]
                .derivatives()[_Xidx] *
            _grad_Xnacl_qp[_qp];
        (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_gas_fluid_component] -=
            _fsp[_aqueous_phase_number]
                .mass_fraction[_aqueous_fluid_component]
                .derivatives()[_Xidx] *
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

template class PorousFlowFluidStateTempl<false>;
template class PorousFlowFluidStateTempl<true>;
