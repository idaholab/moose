//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPorousFlowAquiferBC.h"
#include "PorousFlowDictator.h"

registerADMooseObject("PorousFlowApp", FVPorousFlowAquiferBC);

InputParameters
FVPorousFlowAquiferBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription(
      "Applies a Robin (aquifer) boundary condition: flux = conductance * (P_model - "
      "P_aquifer(z)), where P_aquifer is the far-field aquifer pressure. Using aquifer_head "
      "automatically yields zero flux on any hydrostatic boundary whose model "
      "head equals the aquifer head, even when the boundary has vertical extent. "
      "FV analogue of PorousFlowAquiferBC.");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator",
                                          "The PorousFlowDictator UserObject");
  params.addParam<unsigned int>("phase", 0, "The fluid phase for this BC");
  params.addParam<unsigned int>("fluid_component", 0, "The fluid component for this BC");
  params.addRequiredParam<RealVectorValue>(
      "gravity",
      "Gravitational acceleration vector (m/s^2), e.g. '0 0 -9.81'. "
      "The elevation of the boundary cell is computed as the component of the "
      "cell-centroid position vector in the direction opposite to gravity.");
  params.addParam<Real>("aquifer_head",
                        "Far-field hydraulic head of the aquifer (m above model datum). "
                        "P_aq(z) = rho * |g| * (aquifer_head - z), where rho is "
                        "the fluid density in the boundary cell. "
                        "Mutually exclusive with aquifer_pressure_at_datum.");
  params.addParam<Real>("aquifer_pressure_at_datum",
                        "Far-field aquifer pressure at datum_elevation (Pa). "
                        "P_aq(z) = aquifer_pressure_at_datum "
                        "        + rho * |g| * (datum_elevation - z), where rho is "
                        "the fluid density in the boundary cell. "
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
      "rho is the fluid density, and mu is the fluid viscosity in the boundary cell.");
  params.addRangeCheckedParam<Real>(
      "aquifer_permeability",
      "aquifer_permeability > 0",
      "Permeability of the material between the boundary and the far-field aquifer (m^2), "
      "used as k_nn in the conductance formula of the aquifer_pressure_at_datum formulation. "
      "If not supplied, the boundary permeability projected onto the boundary normal is used, "
      "which is appropriate when the aquifer is a continuation of the boundary material.");
  return params;
}

FVPorousFlowAquiferBC::FVPorousFlowAquiferBC(const InputParameters & params)
  : FVFluxBC(params),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_dictator.numPhases()),
    _phase(getParam<unsigned int>("phase")),
    _fluid_component(getParam<unsigned int>("fluid_component")),
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
    _density(getADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _density_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _viscosity(getADMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _viscosity_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _relperm(getADMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_qp")),
    _relperm_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_qp")),
    _mass_fractions(
        getADMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp")),
    _mass_fractions_neighbor(
        getNeighborADMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp")),
    _permeability(getADMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _permeability_neighbor(
        getNeighborADMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _pressure(getADMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _pressure_neighbor(
        getNeighborADMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp"))
{
  if (_phase >= _num_phases)
    paramError(
        "phase",
        "The Dictator proclaims that the maximum fluid phase index in this simulation is ",
        _num_phases - 1,
        " whereas you have used ",
        _phase,
        ". Remember that indexing starts at 0. The Dictator does not take such mistakes lightly.");

  if (_fluid_component >= _dictator.numComponents())
    paramError(
        "fluid_component",
        "The Dictator proclaims that the maximum fluid component index in this simulation is ",
        _dictator.numComponents() - 1,
        " whereas you have used ",
        _fluid_component,
        ". Remember that indexing starts at 0.");

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

ADReal
FVPorousFlowAquiferBC::computeQpResidual()
{
  const bool out_of_elem = (_face_type == FaceInfo::VarFaceNeighbors::ELEM);

  // Elevation of the boundary-cell centroid: component in the direction opposite to
  // gravity.  The aquifer pressure is evaluated at the same elevation as the cell
  // pressure so that hydrostatic equilibrium gives exactly zero flux on boundaries
  // of any orientation.
  const Point & centroid =
      out_of_elem ? _face_info->elemCentroid() : _face_info->neighborCentroid();
  const Real z = (_g > 0.0) ? -centroid * (_gravity / _g) : 0.0;

  const auto & rho = out_of_elem ? _density[_qp][_phase] : _density_neighbor[_qp][_phase];
  const auto & p = out_of_elem ? _pressure[_qp][_phase] : _pressure_neighbor[_qp][_phase];

  // Far-field aquifer pressure at the cell-centroid elevation, and the conductance
  ADReal p_aq, conductance;
  if (_use_head_form)
  {
    p_aq = rho * _g * (_aquifer_head - z);
    conductance = _conductance;
  }
  else
  {
    p_aq = _p_datum + rho * _g * (_datum_elevation - z);
    // Conductance computed from permeability and viscosity: C = rho * k_nn / (mu * L).
    // k_nn is the user-supplied aquifer permeability if given, otherwise the boundary
    // cell permeability projected onto the boundary normal.
    const auto & mu = out_of_elem ? _viscosity[_qp][_phase] : _viscosity_neighbor[_qp][_phase];
    ADReal k_nn;
    if (_use_aquifer_perm)
      k_nn = _aquifer_permeability;
    else
    {
      const auto & perm = out_of_elem ? _permeability[_qp] : _permeability_neighbor[_qp];
      k_nn = (perm * _normal) * _normal;
    }
    conductance = rho * k_nn / (mu * _aquifer_distance);
  }

  const auto & massfrac = out_of_elem ? _mass_fractions[_qp][_phase][_fluid_component]
                                      : _mass_fractions_neighbor[_qp][_phase][_fluid_component];
  const auto & relperm = out_of_elem ? _relperm[_qp][_phase] : _relperm_neighbor[_qp][_phase];

  const auto flux = massfrac * relperm * conductance * (p - p_aq);

  return out_of_elem ? flux : -flux;
}
