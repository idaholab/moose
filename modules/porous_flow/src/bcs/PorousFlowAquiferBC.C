//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAquiferBC.h"

registerMooseObject("PorousFlowApp", PorousFlowAquiferBC);

InputParameters
PorousFlowAquiferBC::validParams()
{
  InputParameters params = PorousFlowSink::validParams();
  params.addClassDescription(
      "Applies a Robin (aquifer) boundary condition: flux = conductance * (P_model - "
      "P_aquifer(z)), where P_aquifer is the far-field aquifer pressure. Using aquifer_head "
      "automatically yields zero flux on any hydrostatic boundary whose model "
      "head equals the aquifer head, even when the boundary has vertical extent.");
  params.addRequiredParam<RealVectorValue>(
      "gravity",
      "Gravitational acceleration vector (m/s^2), e.g. '0 0 -9.81'. "
      "The elevation at each quadrature point is computed as the component of the "
      "position vector in the direction opposite to gravity.");
  params.addParam<Real>("aquifer_head",
                        "Far-field hydraulic head of the aquifer (m above model datum). "
                        "P_aq(z) = rho_nodal * |g| * (aquifer_head - z), where rho_nodal is "
                        "the PorousFlow nodal fluid density at the boundary node. "
                        "Mutually exclusive with aquifer_pressure_at_datum.");
  params.addParam<Real>("aquifer_pressure_at_datum",
                        "Far-field aquifer pressure at datum_elevation (Pa). "
                        "P_aq(z) = aquifer_pressure_at_datum "
                        "        + rho_nodal * |g| * (datum_elevation - z), where rho_nodal is "
                        "the PorousFlow nodal fluid density at the boundary node. "
                        "Mutually exclusive with aquifer_head.");
  params.addParam<Real>("datum_elevation",
                        0.0,
                        "Elevation of the reference point for aquifer_pressure_at_datum (m).");
  params.addParam<Real>(
      "aquifer_conductance",
      "Conductance per unit boundary area (kg/(m^2*Pa*s)). "
      "The mass flux leaving the domain is conductance * (P_model - P_aquifer). "
      "Required when using the aquifer_head formulation. "
      "Can be estimated as rho * k / (mu * L) where k is aquifer permeability (m^2), "
      "mu is fluid viscosity (Pa.s), and L is the distance to the far-field (m).");
  params.addRangeCheckedParam<Real>(
      "aquifer_distance",
      "aquifer_distance > 0",
      "Distance from the boundary to the far-field aquifer (m). "
      "Required when using the aquifer_pressure_at_datum formulation. "
      "The conductance is computed internally as rho * k_nn / (mu * aquifer_distance), "
      "where k_nn is the permeability projected onto the boundary normal, "
      "rho is the nodal fluid density, and mu is the nodal fluid viscosity.");
  params.addRangeCheckedParam<Real>(
      "aquifer_permeability",
      "aquifer_permeability > 0",
      "Permeability of the material between the boundary and the far-field aquifer (m^2), "
      "used as k_nn in the conductance formula of the aquifer_pressure_at_datum formulation. "
      "If not supplied, the boundary permeability projected onto the boundary normal is used, "
      "which is appropriate when the aquifer is a continuation of the boundary material.");
  return params;
}

PorousFlowAquiferBC::PorousFlowAquiferBC(const InputParameters & parameters)
  : PorousFlowSink(parameters),
    _gravity(getParam<RealVectorValue>("gravity")),
    _g(_gravity.norm()),
    _use_head_form(isParamValid("aquifer_head")),
    _aquifer_head(_use_head_form ? getParam<Real>("aquifer_head") : 0.0),
    _p_datum(!_use_head_form && isParamValid("aquifer_pressure_at_datum")
                 ? getParam<Real>("aquifer_pressure_at_datum")
                 : 0.0),
    _datum_elevation(getParam<Real>("datum_elevation")),
    _conductance(_use_head_form && isParamValid("aquifer_conductance")
                     ? getParam<Real>("aquifer_conductance")
                     : 0.0),
    _aquifer_distance(!_use_head_form && isParamValid("aquifer_distance")
                          ? getParam<Real>("aquifer_distance")
                          : 0.0),
    _use_aquifer_perm(isParamValid("aquifer_permeability")),
    _aquifer_permeability(_use_aquifer_perm ? getParam<Real>("aquifer_permeability") : 0.0),
    _pp(getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_nodal")),
    _dpp_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_porepressure_nodal_dvar")),
    _fluid_density_nodal(
        getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_nodal")),
    _dfluid_density_nodal_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_density_nodal_dvar")),
    _permeability_qp(!_use_head_form && !_use_aquifer_perm
                         ? &getMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")
                         : nullptr),
    _fluid_viscosity_nodal(
        !_use_head_form ? &getMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_nodal")
                        : nullptr),
    _dfluid_viscosity_nodal_dvar(!_use_head_form
                                     ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                           "dPorousFlow_viscosity_nodal_dvar")
                                     : nullptr)
{
  if (!_involves_fluid)
    paramError("fluid_phase", "This parameter must be specified.");

  const bool has_head = isParamValid("aquifer_head");
  const bool has_pdat = isParamValid("aquifer_pressure_at_datum");

  if (has_head && has_pdat)
    paramError("aquifer_pressure_at_datum",
               "Specify either aquifer_head or aquifer_pressure_at_datum, not both.");

  if (!has_head && !has_pdat)
    paramError("aquifer_head",
               "Either aquifer_head or aquifer_pressure_at_datum must be specified.");

  if (has_head && !isParamValid("aquifer_conductance"))
    paramError("aquifer_conductance",
               "This parameter must be specified when using the aquifer_head formulation.");

  if (!has_head && !isParamValid("aquifer_distance"))
    paramError("aquifer_distance",
               "This parameter must be specified when using the aquifer_pressure_at_datum "
               "formulation.");

  if (has_head && _use_aquifer_perm)
    paramError("aquifer_permeability",
               "aquifer_permeability is only used with the aquifer_pressure_at_datum "
               "formulation; with aquifer_head the conductance is supplied directly via "
               "aquifer_conductance.");
}

Real
PorousFlowAquiferBC::multiplier() const
{
  // Elevation at the current quadrature point: component in the direction opposite to gravity.
  // This correctly handles 2D (gravity in y) and 3D (gravity in z) models.
  const Real z = (_g > 0.0) ? -_q_point[_qp] * (_gravity / _g) : 0.0;

  // Hydrostatic height factor using the nodal fluid density from PorousFlow's EOS.
  // Using the same density as the rest of PorousFlow makes the correction self-consistent.
  const Real rho = _fluid_density_nodal[_i][_ph];

  // Far-field aquifer pressure at this elevation
  Real p_ref;
  if (_use_head_form)
  {
    p_ref = rho * _g * (_aquifer_head - z);
    return _conductance * (_pp[_i][_ph] - p_ref);
  }
  else
  {
    p_ref = _p_datum + rho * _g * (_datum_elevation - z);
    // Conductance computed from permeability and viscosity: C = rho * k_nn / (mu * L).
    // k_nn is the user-supplied aquifer permeability if given, otherwise the boundary
    // permeability projected onto the boundary normal.
    const Real k_nn = _use_aquifer_perm
                          ? _aquifer_permeability
                          : ((*_permeability_qp)[_qp] * _normals[_qp]) * _normals[_qp];
    const Real mu = (*_fluid_viscosity_nodal)[_i][_ph];
    const Real C = rho * k_nn / (mu * _aquifer_distance);
    return C * (_pp[_i][_ph] - p_ref);
  }
}

Real
PorousFlowAquiferBC::dmultiplier_dvar(unsigned int pvar) const
{
  const Real z = (_g > 0.0) ? -_q_point[_qp] * (_gravity / _g) : 0.0;
  const Real drho_dvar = _dfluid_density_nodal_dvar[_i][_ph][pvar];

  if (_use_head_form)
  {
    // Height factor for the density derivative term (zero on purely horizontal boundaries).
    const Real dz = _aquifer_head - z;
    // Chain rule: d(p_ref)/d(pvar) = d(rho)/d(pvar) * g * dz
    return _conductance * (_dpp_dvar[_i][_ph][pvar] - drho_dvar * _g * dz);
  }
  else
  {
    const Real rho = _fluid_density_nodal[_i][_ph];
    const Real k_nn = _use_aquifer_perm
                          ? _aquifer_permeability
                          : ((*_permeability_qp)[_qp] * _normals[_qp]) * _normals[_qp];
    const Real mu = (*_fluid_viscosity_nodal)[_i][_ph];
    const Real dmu_dvar = (*_dfluid_viscosity_nodal_dvar)[_i][_ph][pvar];

    // P_aq = p_datum + rho * g * (datum - z)
    const Real p_aq = _p_datum + rho * _g * (_datum_elevation - z);
    const Real delta_p = _pp[_i][_ph] - p_aq;
    const Real C = rho * k_nn / (mu * _aquifer_distance);

    // d(C)/d(pvar): chain rule through rho and mu (perm derivs excluded: constant permeability)
    const Real dC_dvar = drho_dvar * k_nn / (mu * _aquifer_distance) -
                         rho * k_nn * dmu_dvar / (mu * mu * _aquifer_distance);

    // d(delta_p)/d(pvar): chain rule through P_model and P_aq
    const Real d_deltap_dvar = _dpp_dvar[_i][_ph][pvar] - drho_dvar * _g * (_datum_elevation - z);

    return dC_dvar * delta_p + C * d_deltap_dvar;
  }
}
