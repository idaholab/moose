//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addRequiredCoupledVar(
      "z",
      "Total mass fractions summed over all phases. The i-th entry is the total mass fraction of "
      "fluid component i + 1; component 0 (the aqueous component) is the remainder. The fluid "
      "state sets how many are required");
  params.addCoupledVar(
      "temperature", 20, "The fluid temperature (C or K, depending on temperature_unit)");
  params.addCoupledVar("xnacl",
                       0,
                       "The salt mass fraction in the brine (kg/kg). Only for fluid states with "
                       "salt, such as PorousFlowBrineCO2");
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
    _fs(this->template getUserObject<PorousFlowFluidStateMultiComponentBase>("fluid_state")),
    _aqueous_phase_number(_fs.aqueousPhaseIndex()),
    _gas_phase_number(_fs.gasPhaseIndex()),
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
    _Tidx(_fs.getTemperatureIndex())
{
  // These are read with coupledGenericDofValue when at_nodes = true, so they must be
  // nodal (Lagrange) variables.  See #33370.
  if (this->_nodal_material)
    this->checkNodalVariables({"gas_porepressure", "z"});

  // Check that the number of phases in the fluidstate class is also provided in the Dictator
  if (_fs.numPhases() != _num_phases)
    mooseError(name(),
               ": only ",
               _fs.numPhases(),
               " phases are allowed. Please check the number of phases entered in the dictator is "
               "correct");

  // Check that the fluid state and the Dictator have the same number of fluid components. The
  // Dictator may omit the salt component when there is no salt conservation equation
  const auto fs_components = _fs.numComponents();
  if (_num_components != fs_components && !(_fs.hasSalt() && _num_components == fs_components - 1))
    mooseError(name(),
               ": the fluid state has ",
               fs_components,
               " fluid components",
               _fs.hasSalt() ? " (or one fewer if salt is not transported)" : "",
               ". Please check the number of fluid components entered in the dictator is correct");

  // Check that one total mass fraction is supplied for every fluid component other than the
  // aqueous component and salt
  if (_num_Z_vars != _fs.numTotalMassFractions())
    this->paramError("z",
                     "The fluid state '",
                     _fs.name(),
                     "' requires ",
                     _fs.numTotalMassFractions(),
                     " total mass fraction(s), but ",
                     _num_Z_vars,
                     " were supplied");

  // Store the composition passed to the fluid state, and the associated variable numbers: every
  // total mass fraction, followed by the salt mass fraction for fluid states with salt
  for (const auto i : make_range(_num_Z_vars))
  {
    _composition.push_back(_nodal_material ? &this->template coupledGenericDofValue<is_ad>("z", i)
                                           : &this->template coupledGenericValue<is_ad>("z", i));
    _grad_composition_qp.push_back(&this->template coupledGenericGradient<is_ad>("z", i));
    _composition_varnum.push_back(coupled("z", i));
  }

  if (_fs.hasSalt())
  {
    // The salt mass fraction may be an element-local variable, which is read at the qps even
    // when the material is evaluated at the nodes
    const bool is_Xnacl_nodal = isCoupled("xnacl") && getFieldVar("xnacl", 0)->isNodal();
    _composition.push_back(_nodal_material && is_Xnacl_nodal
                               ? &this->template coupledGenericDofValue<is_ad>("xnacl")
                               : &this->template coupledGenericValue<is_ad>("xnacl"));
    _grad_composition_qp.push_back(&this->template coupledGenericGradient<is_ad>("xnacl"));
    _composition_varnum.push_back(coupled("xnacl"));
  }
  else if (this->isParamSetByUser("xnacl"))
    this->paramError("xnacl",
                     "The fluid state '",
                     _fs.name(),
                     "' has no salt, so a salt mass fraction cannot be supplied");

  for (const auto varnum : _composition_varnum)
    _composition_var.push_back(
        _dictator.isPorousFlowVariable(varnum) ? _dictator.porousFlowVariableNum(varnum) : 0);
}

template <bool is_ad>
void
PorousFlowFluidStateTempl<is_ad>::thermophysicalProperties()
{
  // The FluidProperty objects use temperature in K
  const GenericReal<is_ad> Tk = _temperature[_qp] + _T_c2k;

  // Gather the composition: the coupled total mass fraction(s), then the salt mass fraction for
  // fluid states with salt
  std::vector<GenericReal<is_ad>> composition(_composition.size());
  for (const auto m : index_range(_composition))
    composition[m] = (*_composition[m])[_qp];

  _fs.clearFluidStateProperties(_fsp);
  _fs.thermophysicalProperties(_gas_porepressure[_qp], Tk, composition, _qp, _fsp);
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
    for (const auto ph : make_range(_num_phases))
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

        for (const auto comp : make_range(_num_components))
          (*_dmass_frac_dvar)[_qp][ph][comp][_pvar] =
              _fsp[ph].mass_fraction[comp].derivatives()[_pidx];
      }

      // For each composition variable (total mass fraction or salt mass fraction) that is a
      // PorousFlow variable, add derivatives wrt it
      for (const auto m : index_range(_composition))
        if (_dictator.isPorousFlowVariable(_composition_varnum[m]))
        {
          const unsigned int idx = _fs.getZIndex(m);
          (*_dporepressure_dvar)[_qp][ph][_composition_var[m]] =
              _fsp[ph].pressure.derivatives()[idx];
          (*_dsaturation_dvar)[_qp][ph][_composition_var[m]] =
              _fsp[ph].saturation.derivatives()[idx];
          (*_dfluid_density_dvar)[_qp][ph][_composition_var[m]] =
              _fsp[ph].density.derivatives()[idx];
          (*_dfluid_viscosity_dvar)[_qp][ph][_composition_var[m]] =
              _fsp[ph].viscosity.derivatives()[idx];
          (*_dfluid_enthalpy_dvar)[_qp][ph][_composition_var[m]] =
              _fsp[ph].enthalpy.derivatives()[idx];
          (*_dfluid_internal_energy_dvar)[_qp][ph][_composition_var[m]] =
              _fsp[ph].internal_energy.derivatives()[idx];

          for (const auto comp : make_range(_num_components))
            (*_dmass_frac_dvar)[_qp][ph][comp][_composition_var[m]] =
                _fsp[ph].mass_fraction[comp].derivatives()[idx];
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

        for (const auto comp : make_range(_num_components))
          (*_dmass_frac_dvar)[_qp][ph][comp][_Tvar] =
              _fsp[ph].mass_fraction[comp].derivatives()[_Tidx];
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

      // Gradients of saturation and porepressure in all phases. The saturation depends on the
      // gas porepressure, every composition variable and the temperature. The derivatives are
      // read from the fluid state rather than _dsaturation_dvar, which only holds derivatives wrt
      // PorousFlow variables, as the salt mass fraction and the temperature are often
      // AuxVariables
      const auto & dsat = _fsp[_gas_phase_number].saturation.derivatives();
      (*_grads_qp)[_qp][_gas_phase_number] = dsat[_pidx] * _gas_gradp_qp[_qp];
      for (const auto m : index_range(_composition))
        (*_grads_qp)[_qp][_gas_phase_number] +=
            dsat[_fs.getZIndex(m)] * (*_grad_composition_qp[m])[_qp];
      (*_grads_qp)[_qp][_gas_phase_number] += dsat[_Tidx] * (*_gradT_qp)[_qp];
      (*_grads_qp)[_qp][_aqueous_phase_number] = -(*_grads_qp)[_qp][_gas_phase_number];

      (*_gradp_qp)[_qp][_gas_phase_number] = _gas_gradp_qp[_qp];
      (*_gradp_qp)[_qp][_aqueous_phase_number] =
          _gas_gradp_qp[_qp] - dpc * (*_grads_qp)[_qp][_aqueous_phase_number];

      // Gradient of the mass fraction of every component in every phase. Each mass fraction
      // depends on the gas porepressure, every composition variable and the temperature
      for (const auto ph : make_range(_num_phases))
        for (const auto comp : make_range(_num_components))
        {
          RealGradient grad_mf =
              _fsp[ph].mass_fraction[comp].derivatives()[_pidx] * _gas_gradp_qp[_qp];
          for (const auto m : index_range(_composition))
            grad_mf += _fsp[ph].mass_fraction[comp].derivatives()[_fs.getZIndex(m)] *
                       (*_grad_composition_qp[m])[_qp];
          grad_mf += _fsp[ph].mass_fraction[comp].derivatives()[_Tidx] * (*_gradT_qp)[_qp];
          (*_grad_mass_frac_qp)[_qp][ph][comp] = grad_mf;
        }

      // Derivatives of gradients wrt variables
      if (_dictator.isPorousFlowVariable(_gas_porepressure_varnum))
      {
        for (const auto ph : make_range(_num_phases))
          (*_dgradp_qp_dgradv)[_qp][ph][_pvar] = 1.0;

        (*_dgradp_qp_dgradv)[_qp][_aqueous_phase_number][_pvar] +=
            -dpc * (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_pvar];

        (*_dgradp_qp_dv)[_qp][_aqueous_phase_number][_pvar] =
            -d2pc * (*_grads_qp)[_qp][_aqueous_phase_number] *
            (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_pvar];
      }

      for (const auto m : index_range(_composition))
        if (_dictator.isPorousFlowVariable(_composition_varnum[m]))
        {
          const auto var = _composition_var[m];
          (*_dgradp_qp_dgradv)[_qp][_aqueous_phase_number][var] =
              -dpc * (*_dsaturation_dvar)[_qp][_aqueous_phase_number][var];

          (*_dgradp_qp_dv)[_qp][_aqueous_phase_number][var] =
              -d2pc * (*_grads_qp)[_qp][_aqueous_phase_number] *
              (*_dsaturation_dvar)[_qp][_aqueous_phase_number][var];
        }

      if (_dictator.isPorousFlowVariable(_temperature_varnum))
      {
        (*_dgradp_qp_dgradv)[_qp][_aqueous_phase_number][_Tvar] =
            -dpc * (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_Tvar];

        (*_dgradp_qp_dv)[_qp][_aqueous_phase_number][_Tvar] =
            -d2pc * (*_grads_qp)[_qp][_aqueous_phase_number] *
            (*_dsaturation_dvar)[_qp][_aqueous_phase_number][_Tvar];
      }
    }
}

template class PorousFlowFluidStateTempl<false>;
template class PorousFlowFluidStateTempl<true>;
