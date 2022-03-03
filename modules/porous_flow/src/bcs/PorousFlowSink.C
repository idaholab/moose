//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowSink.h"

#include "MooseVariable.h"

#include "libmesh/quadrature.h"

#include <iostream>

registerMooseObject("PorousFlowApp", PorousFlowSink);

InputParameters
PorousFlowSink::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addParam<unsigned int>("fluid_phase",
                                "If supplied, then this BC will potentially be a function of fluid "
                                "pressure, and you can use mass_fraction_component, use_mobility, "
                                "use_relperm, use_enthalpy and use_energy.  If not supplied, then "
                                "this BC can only be a function of temperature");
  params.addParam<unsigned int>("mass_fraction_component",
                                "The index corresponding to a fluid "
                                "component.  If supplied, the flux will "
                                "be multiplied by the nodal mass "
                                "fraction for the component");
  params.addParam<bool>("use_mobility",
                        false,
                        "If true, then fluxes are multiplied by "
                        "(density*permeability_nn/viscosity), where the "
                        "'_nn' indicates the component normal to the "
                        "boundary.  In this case bare_flux is measured in "
                        "Pa.m^-1.  This can be used in conjunction with "
                        "other use_*");
  params.addParam<bool>("use_relperm",
                        false,
                        "If true, then fluxes are multiplied by relative "
                        "permeability.  This can be used in conjunction with "
                        "other use_*");
  params.addParam<bool>("use_enthalpy",
                        false,
                        "If true, then fluxes are multiplied by enthalpy.  "
                        "In this case bare_flux is measured in kg.m^-2.s^-1 "
                        "/ (J.kg).  This can be used in conjunction with "
                        "other use_*");
  params.addParam<bool>("use_internal_energy",
                        false,
                        "If true, then fluxes are multiplied by fluid internal energy. "
                        " In this case bare_flux is measured in kg.m^-2.s^-1 / (J.kg). "
                        " This can be used in conjunction with other use_*");
  params.addParam<bool>("use_thermal_conductivity",
                        false,
                        "If true, then fluxes are multiplied by "
                        "thermal conductivity projected onto "
                        "the normal direction.  This can be "
                        "used in conjunction with other use_*");
  params.addParam<FunctionName>(
      "flux_function",
      1.0,
      "The flux.  The flux is OUT of the medium: hence positive values of "
      "this function means this BC will act as a SINK, while negative values "
      "indicate this flux will be a SOURCE.  The functional form is useful "
      "for spatially or temporally varying sinks.  Without any use_*, this "
      "function is measured in kg.m^-2.s^-1 (or J.m^-2.s^-1 for the case "
      "with only heat and no fluids)");
  params.addClassDescription("Applies a flux sink to a boundary.");
  return params;
}

PorousFlowSink::PorousFlowSink(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _involves_fluid(isParamValid("fluid_phase")),
    _ph(_involves_fluid ? getParam<unsigned int>("fluid_phase") : 0),
    _use_mass_fraction(isParamValid("mass_fraction_component")),
    _has_mass_fraction(
        hasMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal") &&
        hasMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
            "dPorousFlow_mass_frac_nodal_dvar")),
    _sp(_use_mass_fraction ? getParam<unsigned int>("mass_fraction_component") : 0),
    _use_mobility(getParam<bool>("use_mobility")),
    _has_mobility(
        hasMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp") &&
        hasMaterialProperty<std::vector<RealTensorValue>>("dPorousFlow_permeability_qp_dvar") &&
        hasMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_nodal") &&
        hasMaterialProperty<std::vector<std::vector<Real>>>(
            "dPorousFlow_fluid_phase_density_nodal_dvar") &&
        hasMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_nodal") &&
        hasMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_viscosity_nodal_dvar")),
    _use_relperm(getParam<bool>("use_relperm")),
    _has_relperm(hasMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_nodal") &&
                 hasMaterialProperty<std::vector<std::vector<Real>>>(
                     "dPorousFlow_relative_permeability_nodal_dvar")),
    _use_enthalpy(getParam<bool>("use_enthalpy")),
    _has_enthalpy(hasMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_nodal") &&
                  hasMaterialProperty<std::vector<std::vector<Real>>>(
                      "dPorousFlow_fluid_phase_enthalpy_nodal_dvar")),
    _use_internal_energy(getParam<bool>("use_internal_energy")),
    _has_internal_energy(
        hasMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_internal_energy_nodal") &&
        hasMaterialProperty<std::vector<std::vector<Real>>>(
            "dPorousFlow_fluid_phase_internal_energy_nodal_dvar")),
    _use_thermal_conductivity(getParam<bool>("use_thermal_conductivity")),
    _has_thermal_conductivity(
        hasMaterialProperty<RealTensorValue>("PorousFlow_thermal_conductivity_qp") &&
        hasMaterialProperty<std::vector<RealTensorValue>>(
            "dPorousFlow_thermal_conductivity_qp_dvar")),
    _m_func(getFunction("flux_function")),
    _permeability(_has_mobility
                      ? &getMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")
                      : nullptr),
    _dpermeability_dvar(_has_mobility ? &getMaterialProperty<std::vector<RealTensorValue>>(
                                            "dPorousFlow_permeability_qp_dvar")
                                      : nullptr),
    _dpermeability_dgradvar(_has_mobility
                                ? &getMaterialProperty<std::vector<std::vector<RealTensorValue>>>(
                                      "dPorousFlow_permeability_qp_dgradvar")
                                : nullptr),
    _fluid_density_node(_has_mobility ? &getMaterialProperty<std::vector<Real>>(
                                            "PorousFlow_fluid_phase_density_nodal")
                                      : nullptr),
    _dfluid_density_node_dvar(_has_mobility ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                                  "dPorousFlow_fluid_phase_density_nodal_dvar")
                                            : nullptr),
    _fluid_viscosity(_has_mobility
                         ? &getMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_nodal")
                         : nullptr),
    _dfluid_viscosity_dvar(_has_mobility ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                               "dPorousFlow_viscosity_nodal_dvar")
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
    _internal_energy(_has_internal_energy ? &getMaterialPropertyByName<std::vector<Real>>(
                                                "PorousFlow_fluid_phase_internal_energy_nodal")
                                          : nullptr),
    _dinternal_energy_dvar(_has_internal_energy
                               ? &getMaterialPropertyByName<std::vector<std::vector<Real>>>(
                                     "dPorousFlow_fluid_phase_internal_energy_nodal_dvar")
                               : nullptr),
    _thermal_conductivity(_has_thermal_conductivity ? &getMaterialProperty<RealTensorValue>(
                                                          "PorousFlow_thermal_conductivity_qp")
                                                    : nullptr),
    _dthermal_conductivity_dvar(_has_thermal_conductivity
                                    ? &getMaterialProperty<std::vector<RealTensorValue>>(
                                          "dPorousFlow_thermal_conductivity_qp_dvar")
                                    : nullptr),
    _perm_derivs(_dictator.usePermDerivs())
{
  if (_involves_fluid && _ph >= _dictator.numPhases())
    paramError("fluid_phase",
               "The Dictator proclaims that the maximum phase index in this simulation is ",
               _dictator.numPhases() - 1,
               " whereas you have used ",
               _ph,
               ". Remember that indexing starts at 0. You must try harder.");

  if (!_involves_fluid && (_use_mass_fraction || _use_mobility || _use_relperm || _use_enthalpy ||
                           _use_internal_energy))
    mooseError("PorousFlowSink: To use_mass_fraction, use_mobility, use_relperm, use_enthalpy or "
               "use_internal_energy, you must provide a fluid phase number");

  if (_use_mass_fraction && _sp >= _dictator.numComponents())
    paramError("mass_fraction_component",
               "The Dictator declares that the maximum fluid component index is ",
               _dictator.numComponents() - 1,
               ", but you have set mass_fraction_component to ",
               _sp,
               ". Remember that indexing starts at 0. Please be assured that the Dictator has "
               "noted your error.");

  if (_use_mass_fraction && !_has_mass_fraction)
    mooseError("PorousFlowSink: You have used the use_mass_fraction flag, but you have no "
               "mass_fraction Material");

  if (_use_mobility && !_has_mobility)
    mooseError("PorousFlowSink: You have used the use_mobility flag, but there are not the "
               "required Materials for this");

  if (_use_relperm && !_has_relperm)
    mooseError(
        "PorousFlowSink: You have used the use_relperm flag, but you have no relperm Material");

  if (_use_enthalpy && !_has_enthalpy)
    mooseError(
        "PorousFlowSink: You have used the use_enthalpy flag, but you have no enthalpy Material");

  if (_use_internal_energy && !_has_internal_energy)
    mooseError("PorousFlowSink: You have used the use_internal_energy flag, but you have no "
               "internal_energy Material");

  if (_use_thermal_conductivity && !_has_thermal_conductivity)
    mooseError("PorousFlowSink: You have used the use_thermal_conductivity flag, but you have no "
               "thermal_conductivity Material");
}

Real
PorousFlowSink::computeQpResidual()
{
  Real flux = _test[_i][_qp] * multiplier();
  if (_use_mobility)
  {
    const Real k =
        ((*_permeability)[_qp] * _normals[_qp]) * _normals[_qp]; // do not upwind permeability
    flux *= (*_fluid_density_node)[_i][_ph] * k / (*_fluid_viscosity)[_i][_ph];
  }
  if (_use_relperm)
    flux *= (*_relative_permeability)[_i][_ph];
  if (_use_mass_fraction)
    flux *= (*_mass_fractions)[_i][_ph][_sp];
  if (_use_enthalpy)
    flux *= (*_enthalpy)[_i][_ph];
  if (_use_internal_energy)
    flux *= (*_internal_energy)[_i][_ph];
  if (_use_thermal_conductivity)
    flux *= ((*_thermal_conductivity)[_qp] * _normals[_qp]) *
            _normals[_qp]; // do not upwind thermal_conductivity

  return flux;
}

Real
PorousFlowSink::computeQpJacobian()
{
  return jac(_var.number());
}

Real
PorousFlowSink::computeQpOffDiagJacobian(unsigned int jvar)
{
  return jac(jvar);
}

Real
PorousFlowSink::jac(unsigned int jvar) const
{
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;
  const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);

  // For _i != _j, note:
  // since the only non-upwinded contribution to the residual is
  // from the permeability and thermal_conductivity, the only contribution
  // of the residual at node _i from changing jvar at node _j is through
  // the derivative of permeability or thermal_conductivity

  Real flux = _test[_i][_qp] * multiplier();
  Real deriv = _test[_i][_qp] * (_i != _j ? 0.0 : dmultiplier_dvar(pvar));

  if (_use_mobility)
  {
    const Real k = ((*_permeability)[_qp] * _normals[_qp]) * _normals[_qp];
    const Real mob = (*_fluid_density_node)[_i][_ph] * k / (*_fluid_viscosity)[_i][_ph];

    Real mobprime = 0.0;
    if (_perm_derivs)
    {
      RealTensorValue ktprime = (*_dpermeability_dvar)[_qp][pvar] * _phi[_j][_qp];
      for (const auto i : make_range(Moose::dim))
        ktprime += (*_dpermeability_dgradvar)[_qp][i][pvar] * _grad_phi[_j][_qp](i);
      const Real kprime = (ktprime * _normals[_qp]) * _normals[_qp];

      mobprime += (*_fluid_density_node)[_i][_ph] * kprime / (*_fluid_viscosity)[_i][_ph];
    }

    mobprime +=
        (_i != _j
             ? 0.0
             : (*_dfluid_density_node_dvar)[_i][_ph][pvar] * k / (*_fluid_viscosity)[_i][_ph] -
                   (*_fluid_density_node)[_i][_ph] * k * (*_dfluid_viscosity_dvar)[_i][_ph][pvar] /
                       std::pow((*_fluid_viscosity)[_i][_ph], 2));
    deriv = mob * deriv + mobprime * flux;
    flux *= mob;
  }
  if (_use_relperm)
  {
    const Real relperm_prime = (_i != _j ? 0.0 : (*_drelative_permeability_dvar)[_i][_ph][pvar]);
    deriv = (*_relative_permeability)[_i][_ph] * deriv + relperm_prime * flux;
    flux *= (*_relative_permeability)[_i][_ph];
  }
  if (_use_mass_fraction)
  {
    const Real mf_prime = (_i != _j ? 0.0 : (*_dmass_fractions_dvar)[_i][_ph][_sp][pvar]);
    deriv = (*_mass_fractions)[_i][_ph][_sp] * deriv + mf_prime * flux;
    flux *= (*_mass_fractions)[_i][_ph][_sp];
  }
  if (_use_enthalpy)
  {
    const Real en_prime = (_i != _j ? 0.0 : (*_denthalpy_dvar)[_i][_ph][pvar]);
    deriv = (*_enthalpy)[_i][_ph] * deriv + en_prime * flux;
    flux *= (*_enthalpy)[_i][_ph];
  }
  if (_use_internal_energy)
  {
    const Real ie_prime = (_i != _j ? 0.0 : (*_dinternal_energy_dvar)[_i][_ph][pvar]);
    deriv = (*_internal_energy)[_i][_ph] * deriv + ie_prime * flux;
    flux *= (*_internal_energy)[_i][_ph];
  }
  if (_use_thermal_conductivity)
  {
    const Real tc = ((*_thermal_conductivity)[_qp] * _normals[_qp]) * _normals[_qp];
    const RealTensorValue tctprime = (*_dthermal_conductivity_dvar)[_qp][pvar] * _phi[_j][_qp];
    const Real tcprime = (tctprime * _normals[_qp]) * _normals[_qp];
    deriv = tc * deriv + tcprime * flux;
    // don't need this: flux *= tc;
  }
  return deriv;
}

Real
PorousFlowSink::multiplier() const
{
  return _m_func.value(_t, _q_point[_qp]);
}

Real
PorousFlowSink::dmultiplier_dvar(unsigned int /*pvar*/) const
{
  return 0.0;
}
