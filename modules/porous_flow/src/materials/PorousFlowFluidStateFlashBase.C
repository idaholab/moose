//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateFlashBase.h"
#include "PorousFlowCapillaryPressure.h"

template <>
InputParameters
validParams<PorousFlowFluidStateFlashBase>()
{
  InputParameters params = validParams<PorousFlowVariableBase>();
  params.addRequiredCoupledVar("gas_porepressure",
                               "Variable that is the porepressure of the gas phase");
  params.addRequiredCoupledVar("z", "Total mass fraction of component i summed over all phases");
  MooseEnum unit_choice("Kelvin=0 Celsius=1", "Kelvin");
  params.addParam<MooseEnum>(
      "temperature_unit", unit_choice, "The unit of the temperature variable");
  params.addRequiredParam<UserObjectName>("capillary_pressure",
                                          "Name of the UserObject defining the capillary pressure");
  params.addRequiredParam<UserObjectName>("fluid_state", "Name of the FluidState UserObject");
  params.addClassDescription("Base class for fluid state calculations using persistent primary "
                             "variables and a vapor-liquid flash");
  return params;
}

PorousFlowFluidStateFlashBase::PorousFlowFluidStateFlashBase(const InputParameters & parameters)
  : PorousFlowVariableBase(parameters),

    _gas_porepressure(_nodal_material ? coupledNodalValue("gas_porepressure")
                                      : coupledValue("gas_porepressure")),
    _gas_gradp_qp(coupledGradient("gas_porepressure")),
    _gas_porepressure_varnum(coupled("gas_porepressure")),
    _pvar(_dictator.isPorousFlowVariable(_gas_porepressure_varnum)
              ? _dictator.porousFlowVariableNum(_gas_porepressure_varnum)
              : 0),

    _num_z_vars(coupledComponents("z")),

    _fs_base(getUserObject<PorousFlowFluidStateBase>("fluid_state")),
    _aqueous_phase_number(_fs_base.aqueousPhaseIndex()),
    _gas_phase_number(_fs_base.gasPhaseIndex()),
    _aqueous_fluid_component(_fs_base.aqueousComponentIndex()),
    _gas_fluid_component(_fs_base.gasComponentIndex()),

    _temperature(_nodal_material ? getMaterialProperty<Real>("PorousFlow_temperature_nodal")
                                 : getMaterialProperty<Real>("PorousFlow_temperature_qp")),
    _gradT_qp(getMaterialProperty<RealGradient>("PorousFlow_grad_temperature_qp")),
    _dtemperature_dvar(
        _nodal_material
            ? getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")
            : getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar")),

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
    _saturation_old(_nodal_material
                        ? getMaterialPropertyOld<std::vector<Real>>("PorousFlow_saturation_nodal")
                        : getMaterialPropertyOld<std::vector<Real>>("PorousFlow_saturation_qp")),

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

    _T_c2k(getParam<MooseEnum>("temperature_unit") == 0 ? 0.0 : 273.15),
    _is_initqp(false),
    _pc_uo(getUserObject<PorousFlowCapillaryPressure>("capillary_pressure"))
{
  // Only two phases are possible in the fluidstate classes
  if (_fs_base.numPhases() != _num_phases)
    mooseError("Only ",
               _fs_base.numPhases(),
               " phases are allowed in ",
               _name,
               ". Please check the number of phases entered in the dictator is correct");

  // Check that the number of total mass fractions provided as primary variables is correct
  if (_num_z_vars != _num_components - 1)
    mooseError("The number of supplied mass fraction variables should be ",
               _num_components - 1,
               " in ",
               _name,
               " but ",
               _num_z_vars,
               " are supplied");

  // Store all total mass fractions and associated variable numbers
  _z.resize(_num_z_vars);
  _gradz_qp.resize(_num_z_vars);
  _z_varnum.resize(_num_z_vars);
  _zvar.resize(_num_z_vars);

  for (unsigned int i = 0; i < _num_z_vars; ++i)
  {
    _z[i] = (_nodal_material ? &coupledNodalValue("z", i) : &coupledValue("z", i));
    _gradz_qp[i] = &coupledGradient("z", i);
    _z_varnum[i] = coupled("z", i);
    _zvar[i] = (_dictator.isPorousFlowVariable(_z_varnum[i])
                    ? _dictator.porousFlowVariableNum(_z_varnum[i])
                    : 0);
  }

  // Set the size of the FluidStateProperties vector
  _fsp.resize(_num_phases, FluidStateProperties(_num_components));
}

void
PorousFlowFluidStateFlashBase::initQpStatefulProperties()
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

    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      _saturation[_qp][ph] = _fsp[ph].saturation;
      _porepressure[_qp][ph] = _fsp[ph].pressure;
      _fluid_density[_qp][ph] = _fsp[ph].density;
      _fluid_viscosity[_qp][ph] = _fsp[ph].viscosity;
      _mass_frac[_qp][ph] = _fsp[ph].mass_fraction;
    }
  }
}

void
PorousFlowFluidStateFlashBase::computeQpProperties()
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
    _saturation[_qp][ph] = _fsp[ph].saturation;
    _porepressure[_qp][ph] = _fsp[ph].pressure;
    _fluid_density[_qp][ph] = _fsp[ph].density;
    _fluid_viscosity[_qp][ph] = _fsp[ph].viscosity;
    _mass_frac[_qp][ph] = _fsp[ph].mass_fraction;
  }

  // Derivative of saturation wrt variables
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _dsaturation_dvar[_qp][ph][_zvar[0]] = _fsp[ph].dsaturation_dz;
    _dsaturation_dvar[_qp][ph][_pvar] = _fsp[ph].dsaturation_dp;
  }
  // Derivative of capillary pressure
  Real dpc = _pc_uo.dCapillaryPressure(_fsp[_aqueous_phase_number].saturation);

  // Derivative of porepressure wrt variables
  if (_dictator.isPorousFlowVariable(_gas_porepressure_varnum))
  {
    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      _dporepressure_dvar[_qp][ph][_pvar] = 1.0;
      if (!_nodal_material)
        (*_dgradp_qp_dgradv)[_qp][ph][_pvar] = 1.0;
    }

    if (!_nodal_material)
    {
      (*_dgradp_qp_dgradv)[_qp][_aqueous_phase_number][_pvar] +=
          -dpc * _dsaturation_dvar[_qp][_aqueous_phase_number][_pvar];
      (*_dgradp_qp_dgradv)[_qp][_aqueous_phase_number][_zvar[0]] =
          -dpc * _dsaturation_dvar[_qp][_aqueous_phase_number][_zvar[0]];
    }

    // The aqueous phase porepressure is also a function of liquid saturation,
    // which depends on both gas porepressure and z
    _dporepressure_dvar[_qp][_aqueous_phase_number][_pvar] +=
        -dpc * _dsaturation_dvar[_qp][_aqueous_phase_number][_pvar];
    _dporepressure_dvar[_qp][_aqueous_phase_number][_zvar[0]] =
        -dpc * _dsaturation_dvar[_qp][_aqueous_phase_number][_zvar[0]];
  }

  // Calculate derivatives of material properties wrt primary variables
  // Derivative of z wrt variables
  std::vector<Real> dz_dvar;
  dz_dvar.assign(_num_pf_vars, 0.0);
  if (_dictator.isPorousFlowVariable(_z_varnum[0]))
    dz_dvar[_zvar[0]] = 1.0;

  // Derivatives of properties wrt primary variables
  for (unsigned int v = 0; v < _num_pf_vars; ++v)
  {
    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      // Derivative of density in each phase
      _dfluid_density_dvar[_qp][ph][v] = _fsp[ph].ddensity_dp * _dporepressure_dvar[_qp][ph][v];
      _dfluid_density_dvar[_qp][ph][v] += _fsp[ph].ddensity_dT * _dtemperature_dvar[_qp][v];
      _dfluid_density_dvar[_qp][ph][v] += _fsp[ph].ddensity_dz * dz_dvar[v];

      // Derivative of viscosity in each phase
      _dfluid_viscosity_dvar[_qp][ph][v] = _fsp[ph].dviscosity_dp * _dporepressure_dvar[_qp][ph][v];
      _dfluid_viscosity_dvar[_qp][ph][v] += _fsp[ph].dviscosity_dT * _dtemperature_dvar[_qp][v];
      _dfluid_viscosity_dvar[_qp][ph][v] += _fsp[ph].dviscosity_dz * dz_dvar[v];
    }
  }

  // The derivative of the mass fractions for each fluid component in each phase.
  // Note: these are all calculated in terms of gas pressuse, so there is no
  // capillary pressure effect, and hence no need to multiply by _dporepressure_dvar
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    for (unsigned int comp = 0; comp < _num_components; ++comp)
    {
      _dmass_frac_dvar[_qp][ph][comp][_pvar] = _fsp[ph].dmass_fraction_dp[comp];
      _dmass_frac_dvar[_qp][ph][comp][_zvar[0]] =
          _fsp[ph].dmass_fraction_dz[comp] * dz_dvar[_zvar[0]];
    }
  }

  // If the material properties are being evaluated at the qps, calculate the
  // gradients as well. Note: only nodal properties are evaluated in
  // initQpStatefulProperties(), so no need to check _is_initqp flag for qp
  // properties
  if (!_nodal_material)
  {
    // Second derivative of capillary pressure
    Real d2pc = _pc_uo.d2CapillaryPressure(_fsp[_aqueous_phase_number].saturation);

    (*_grads_qp)[_qp][_aqueous_phase_number] =
        _dsaturation_dvar[_qp][_aqueous_phase_number][_pvar] * _gas_gradp_qp[_qp] +
        _dsaturation_dvar[_qp][_aqueous_phase_number][_zvar[0]] * (*_gradz_qp[0])[_qp];
    (*_grads_qp)[_qp][_gas_phase_number] = -(*_grads_qp)[_qp][_aqueous_phase_number];

    (*_gradp_qp)[_qp][_gas_phase_number] = _gas_gradp_qp[_qp];
    (*_gradp_qp)[_qp][_aqueous_phase_number] =
        _gas_gradp_qp[_qp] - dpc * (*_grads_qp)[_qp][_aqueous_phase_number];

    (*_dgradp_qp_dv)[_qp][_aqueous_phase_number][_pvar] =
        -d2pc * (*_grads_qp)[_qp][_aqueous_phase_number] *
        _dsaturation_dvar[_qp][_aqueous_phase_number][_pvar];
    (*_dgradp_qp_dv)[_qp][_aqueous_phase_number][_zvar[0]] =
        -d2pc * (*_grads_qp)[_qp][_aqueous_phase_number] *
        _dsaturation_dvar[_qp][_aqueous_phase_number][_zvar[0]];

    (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_aqueous_fluid_component] =
        _fsp[_aqueous_phase_number].dmass_fraction_dp[_aqueous_fluid_component] *
            _gas_gradp_qp[_qp] +
        _fsp[_aqueous_phase_number].dmass_fraction_dz[_aqueous_fluid_component] *
            (*_gradz_qp[0])[_qp];
    (*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_gas_fluid_component] =
        -(*_grad_mass_frac_qp)[_qp][_aqueous_phase_number][_aqueous_fluid_component];
    (*_grad_mass_frac_qp)[_qp][_gas_phase_number][_aqueous_fluid_component] =
        _fsp[_gas_phase_number].dmass_fraction_dp[_aqueous_fluid_component] * _gas_gradp_qp[_qp] +
        _fsp[_gas_phase_number].dmass_fraction_dz[_aqueous_fluid_component] * (*_gradz_qp[0])[_qp];
    (*_grad_mass_frac_qp)[_qp][_gas_phase_number][_gas_fluid_component] =
        -(*_grad_mass_frac_qp)[_qp][_gas_phase_number][_aqueous_fluid_component];
  }
}

void
PorousFlowFluidStateFlashBase::setMaterialVectorSize() const
{
  _fluid_density[_qp].assign(_num_phases, 0.0);
  _fluid_viscosity[_qp].assign(_num_phases, 0.0);
  _mass_frac[_qp].resize(_num_phases);

  // Derivatives and gradients are not required in initQpStatefulProperties
  if (!_is_initqp)
  {
    _dfluid_density_dvar[_qp].resize(_num_phases);
    _dfluid_viscosity_dvar[_qp].resize(_num_phases);
    _dmass_frac_dvar[_qp].resize(_num_phases);

    if (!_nodal_material)
      (*_grad_mass_frac_qp)[_qp].resize(_num_phases);

    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      _dfluid_density_dvar[_qp][ph].assign(_num_pf_vars, 0.0);
      _dfluid_viscosity_dvar[_qp][ph].assign(_num_pf_vars, 0.0);
      _dmass_frac_dvar[_qp][ph].resize(_num_components);

      for (unsigned int comp = 0; comp < _num_components; ++comp)
        _dmass_frac_dvar[_qp][ph][comp].assign(_num_pf_vars, 0.0);

      if (!_nodal_material)
        (*_grad_mass_frac_qp)[_qp][ph].assign(_num_components, RealGradient());
    }
  }
}
