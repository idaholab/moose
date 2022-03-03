//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowOutflowBC.h"

#include "MooseVariable.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature.h"

registerMooseObject("PorousFlowApp", PorousFlowOutflowBC);

InputParameters
PorousFlowOutflowBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addRequiredParam<RealVectorValue>("gravity",
                                           "Gravitational acceleration vector downwards (m/s^2)");
  MooseEnum flux_type_enum("fluid heat", "fluid");
  params.addParam<MooseEnum>(
      "flux_type",
      flux_type_enum,
      "The type of boundary condition to apply.  'fluid' means this boundary condition will allow "
      "a fluid component to flow freely from the boundary.  'heat' means this boundary condition "
      "will allow heat energy to flow freely from the boundary");
  params.addParam<unsigned int>(
      "mass_fraction_component",
      0,
      "The index corresponding to a fluid "
      "component.  If supplied, the residual contribution will be "
      "multiplied by the mass fraction, corresponding to allowing the "
      "given mass fraction to flow freely from the boundary.  This is ignored if flux_type = heat");
  params.addParam<bool>("multiply_by_density",
                        true,
                        "If true, this BC represents mass flux.  If false, it represents volume "
                        "flux.  User input of this flag is ignored if flux_type = heat as "
                        "multiply_by_density should always be true in that case");
  params.addParam<bool>(
      "include_relperm",
      true,
      "If true, the Darcy flux will include the relative permeability.  If false, the relative "
      "permeability will not be used, which must only be used for fully-saturated situations "
      "where there is no notion of relative permeability");
  params.addParam<Real>(
      "multiplier",
      1.0,
      "Multiply the flux by this number.  This is mainly used for testing purposes");
  params.addParamNamesToGroup("multiplier", "Advanced");
  params.addClassDescription(
      "Applies an 'outflow' boundary condition, which allows fluid components or heat energy to "
      "flow freely out of the boundary as if it weren't there.  This is fully upwinded");
  return params;
}

PorousFlowOutflowBC::PorousFlowOutflowBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_dictator.numPhases()),
    _perm_derivs(_dictator.usePermDerivs()),
    _flux_type(getParam<MooseEnum>("flux_type").getEnum<FluxTypeChoiceEnum>()),
    _sp(getParam<unsigned>("mass_fraction_component")),
    _multiply_by_density(
        _flux_type == FluxTypeChoiceEnum::FLUID ? getParam<bool>("multiply_by_density") : true),
    _include_relperm(getParam<bool>("include_relperm")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _multiplier(getParam<Real>("multiplier")),
    _grad_p(getMaterialProperty<std::vector<RealGradient>>("PorousFlow_grad_porepressure_qp")),
    _dgrad_p_dgrad_var(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_grad_porepressure_qp_dgradvar")),
    _dgrad_p_dvar(getMaterialProperty<std::vector<std::vector<RealGradient>>>(
        "dPorousFlow_grad_porepressure_qp_dvar")),
    _fluid_density_qp(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _dfluid_density_qp_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_density_qp_dvar")),
    _permeability(getMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _dpermeability_dvar(
        getMaterialProperty<std::vector<RealTensorValue>>("dPorousFlow_permeability_qp_dvar")),
    _dpermeability_dgradvar(getMaterialProperty<std::vector<std::vector<RealTensorValue>>>(
        "dPorousFlow_permeability_qp_dgradvar")),
    _fluid_viscosity(getMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_nodal")),
    _dfluid_viscosity_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_viscosity_nodal_dvar")),

    _has_density(hasMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_nodal") &&
                 hasMaterialProperty<std::vector<std::vector<Real>>>(
                     "dPorousFlow_fluid_phase_density_nodal_dvar")),
    _has_mass_fraction(
        hasMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal") &&
        hasMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
            "dPorousFlow_mass_frac_nodal_dvar")),
    _has_relperm(hasMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_nodal") &&
                 hasMaterialProperty<std::vector<std::vector<Real>>>(
                     "dPorousFlow_relative_permeability_nodal_dvar")),
    _has_enthalpy(hasMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_nodal") &&
                  hasMaterialProperty<std::vector<std::vector<Real>>>(
                      "dPorousFlow_fluid_phase_enthalpy_nodal_dvar")),
    _has_thermal_conductivity(
        hasMaterialProperty<RealTensorValue>("PorousFlow_thermal_conductivity_qp") &&
        hasMaterialProperty<std::vector<RealTensorValue>>(
            "dPorousFlow_thermal_conductivity_qp_dvar")),
    _has_t(hasMaterialProperty<RealGradient>("PorousFlow_grad_temperature_qp") &&
           hasMaterialProperty<std::vector<RealGradient>>("dPorousFlow_grad_temperature_qp_dvar") &&
           hasMaterialProperty<std::vector<Real>>("dPorousFlow_grad_temperature_qp_dgradvar")),

    _fluid_density_node(_has_density ? &getMaterialProperty<std::vector<Real>>(
                                           "PorousFlow_fluid_phase_density_nodal")
                                     : nullptr),
    _dfluid_density_node_dvar(_has_density ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                                 "dPorousFlow_fluid_phase_density_nodal_dvar")
                                           : nullptr),
    _relative_permeability(_has_relperm ? &getMaterialProperty<std::vector<Real>>(
                                              "PorousFlow_relative_permeability_nodal")
                                        : nullptr),
    _drelative_permeability_dvar(_has_relperm
                                     ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                           "dPorousFlow_relative_permeability_nodal_dvar")
                                     : nullptr),
    _mass_fractions(_has_mass_fraction ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                             "PorousFlow_mass_frac_nodal")
                                       : nullptr),
    _dmass_fractions_dvar(_has_mass_fraction
                              ? &getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
                                    "dPorousFlow_mass_frac_nodal_dvar")
                              : nullptr),
    _enthalpy(_has_enthalpy ? &getMaterialPropertyByName<std::vector<Real>>(
                                  "PorousFlow_fluid_phase_enthalpy_nodal")
                            : nullptr),
    _denthalpy_dvar(_has_enthalpy ? &getMaterialPropertyByName<std::vector<std::vector<Real>>>(
                                        "dPorousFlow_fluid_phase_enthalpy_nodal_dvar")
                                  : nullptr),
    _thermal_conductivity(_has_thermal_conductivity ? &getMaterialProperty<RealTensorValue>(
                                                          "PorousFlow_thermal_conductivity_qp")
                                                    : nullptr),
    _dthermal_conductivity_dvar(_has_thermal_conductivity
                                    ? &getMaterialProperty<std::vector<RealTensorValue>>(
                                          "dPorousFlow_thermal_conductivity_qp_dvar")
                                    : nullptr),
    _grad_t(_has_t ? &getMaterialProperty<RealGradient>("PorousFlow_grad_temperature_qp")
                   : nullptr),
    _dgrad_t_dvar(_has_t ? &getMaterialProperty<std::vector<RealGradient>>(
                               "dPorousFlow_grad_temperature_qp_dvar")
                         : nullptr),
    _dgrad_t_dgradvar(
        _has_t ? &getMaterialProperty<std::vector<Real>>("dPorousFlow_grad_temperature_qp_dgradvar")
               : nullptr)
{
  if (_flux_type == FluxTypeChoiceEnum::FLUID && _sp >= _dictator.numComponents())
    paramError("mass_fraction_component",
               "The Dictator declares that the maximum fluid component index is ",
               _dictator.numComponents() - 1,
               ", but you have set mass_fraction_component to ",
               _sp,
               ". Remember that indexing starts at 0. Please be assured that the Dictator has "
               "noted your error.");

  if (_multiply_by_density && !_has_density)
    mooseError("PorousFlowOutflowBC: You requested to multiply_by_density, but you have no nodal "
               "fluid density Material");
  if (_include_relperm && !_has_relperm)
    mooseError("PorousFlowOutflowBC: You requested to include the relative permeability, but you "
               "have no nodal relative permeability Material");
  if (_flux_type == FluxTypeChoiceEnum::FLUID && !_has_mass_fraction)
    mooseError(
        "PorousFlowOutflowBC: For flux_type = fluid, you need a nodal mass-fraction Material");
  if (_flux_type == FluxTypeChoiceEnum::HEAT && !_has_enthalpy)
    mooseError(
        "PorousFlowOutflowBC: For flux_type = heat, you need a nodal fluid enthalpy Material");
  if (_flux_type == FluxTypeChoiceEnum::HEAT && !_has_thermal_conductivity)
    mooseError(
        "PorousFlowOutflowBC: For flux_type = heat, you need a thermal conductivity Material");
  if (_flux_type == FluxTypeChoiceEnum::HEAT && !_has_t)
    mooseError("PorousFlowOutflowBC: For flux_type = heat, you need a temperature Material");
}

Real
PorousFlowOutflowBC::computeQpResidual()
{
  Real advective_term = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
    advective_term -= darcy(ph) * mobility(ph) * prefactor(ph);

  if (_flux_type == FluxTypeChoiceEnum::FLUID)
    return _test[_i][_qp] * advective_term * _multiplier;

  const Real conduction_term = -_normals[_qp] * ((*_thermal_conductivity)[_qp] * (*_grad_t)[_qp]);
  return _test[_i][_qp] * (conduction_term + advective_term) * _multiplier;
}

Real
PorousFlowOutflowBC::computeQpJacobian()
{
  return jac(_var.number());
}

Real
PorousFlowOutflowBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  return jac(jvar);
}

Real
PorousFlowOutflowBC::jac(unsigned int jvar) const
{
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;
  const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);

  // For _i != _j, note:
  // since the only non-upwinded contribution to the residual is
  // from Darcy and thermal_conductivity, the only contribution
  // of the residual at node _i from changing jvar at node _j is through
  // the derivative of Darcy or thermal_conductivity

  Real advective_term = 0.0;
  Real advective_term_prime = 0.0;
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    Real darcyprime =
        _normals[_qp] *
        (_permeability[_qp] * (_grad_phi[_j][_qp] * _dgrad_p_dgrad_var[_qp][ph][pvar] +
                               _dgrad_p_dvar[_qp][ph][pvar] * _phi[_j][_qp] -
                               _dfluid_density_qp_dvar[_qp][ph][pvar] * _phi[_j][_qp] * _gravity));
    if (_perm_derivs)
    {
      darcyprime += _normals[_qp] * (_dpermeability_dvar[_qp][pvar] * _phi[_j][_qp] *
                                     (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity));
      for (const auto i : make_range(Moose::dim))
        darcyprime +=
            _normals[_qp] * (_dpermeability_dgradvar[_qp][i][pvar] * _grad_phi[_j][_qp](i) *
                             (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity));
    }
    if (_i != _j)
      advective_term_prime -= prefactor(ph) * mobility(ph) * darcyprime;
    else
    {
      const Real dar = darcy(ph);

      Real mob = 1.0 / _fluid_viscosity[_i][ph];
      Real mobprime =
          -_dfluid_viscosity_dvar[_i][ph][pvar] / Utility::pow<2>(_fluid_viscosity[_i][ph]);
      if (_multiply_by_density)
      {
        const Real densityprime = (*_dfluid_density_node_dvar)[_i][ph][pvar];
        mobprime = densityprime * mob + (*_fluid_density_node)[_i][ph] * mobprime;
        mob *= (*_fluid_density_node)[_i][ph];
      }
      if (_include_relperm)
      {
        const Real relpermprime = (*_drelative_permeability_dvar)[_i][ph][pvar];
        mobprime = relpermprime * mob + (*_relative_permeability)[_i][ph] * mobprime;
        mob *= (*_relative_permeability)[_i][ph];
      }

      const Real pre = prefactor(ph);
      const Real preprime =
          (_flux_type == FluxTypeChoiceEnum::FLUID ? (*_dmass_fractions_dvar)[_i][ph][_sp][pvar]
                                                   : (*_denthalpy_dvar)[_i][ph][pvar]);

      advective_term -= pre * mob * dar;
      advective_term_prime -= preprime * mob * dar + pre * mobprime * dar + pre * mob * darcyprime;
    }
  }
  if (_flux_type == FluxTypeChoiceEnum::FLUID)
    return _test[_i][_qp] * advective_term_prime * _multiplier;

  const Real conduction_term_prime =
      -_normals[_qp] *
          ((*_dthermal_conductivity_dvar)[_qp][pvar] * (*_grad_t)[_qp] +
           (*_thermal_conductivity)[_qp] * (*_dgrad_t_dvar)[_qp][pvar]) *
          _phi[_j][_qp] -
      _normals[_qp] *
          ((*_thermal_conductivity)[_qp] * (*_dgrad_t_dgradvar)[_qp][pvar] * _grad_phi[_j][_qp]);
  return _test[_i][_qp] * (conduction_term_prime + advective_term_prime) * _multiplier;
}

Real
PorousFlowOutflowBC::darcy(unsigned ph) const
{
  return _normals[_qp] *
         (_permeability[_qp] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity));
}

Real
PorousFlowOutflowBC::mobility(unsigned ph) const
{
  return (_multiply_by_density ? (*_fluid_density_node)[_i][ph] : 1.0) *
         (_include_relperm ? (*_relative_permeability)[_i][ph] : 1.0) / _fluid_viscosity[_i][ph];
}

Real
PorousFlowOutflowBC::prefactor(unsigned ph) const
{
  return (_flux_type == FluxTypeChoiceEnum::FLUID ? (*_mass_fractions)[_i][ph][_sp]
                                                  : (*_enthalpy)[_i][ph]);
}
