//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowSink.h"

/**
 * Robin (aquifer) boundary condition for PorousFlow.
 *
 * Applies a flux proportional to the difference between the model pore pressure
 * and a far-field aquifer pressure that is corrected for elevation:
 *
 *   flux = conductance * (P_model - P_aquifer(z))   [positive = fluid leaves domain]
 *
 * The aquifer reference pressure at elevation z is computed from either:
 *  (a) a hydraulic head:  P_aq(z) = rho_nodal * g * (aquifer_head - z)
 *  (b) a pressure at datum: P_aq(z) = aquifer_pressure_at_datum
 *                                     + rho_nodal * g * (datum_elevation - z)
 *
 * rho_nodal is the PorousFlow nodal fluid density at the current node, taken from
 * the PorousFlow_fluid_phase_density_nodal material property.  This makes the
 * hydrostatic correction self-consistent with PorousFlow's own fluid EOS.
 *
 * Conductance specification:
 *  (a) head form:     user supplies aquifer_conductance [kg/(m^2 Pa s)]
 *  (b) pressure form: conductance is computed as rho * k_nn / (mu * L), where
 *      k_nn is the permeability normal to the boundary, mu is the fluid viscosity,
 *      and L = aquifer_distance is the distance to the far-field [m].  By default
 *      k_nn comes from the boundary permeability material property; the optional
 *      aquifer_permeability parameter overrides it when the material between the
 *      boundary and the far-field aquifer differs from the boundary material.
 *
 * Using hydraulic head automatically yields zero flux everywhere on a boundary
 * whose model pressure is at hydrostatic equilibrium with the aquifer (for the
 * same fluid EOS), even when the boundary has vertical extent.
 */
class PorousFlowAquiferBC : public PorousFlowSink
{
public:
  static InputParameters validParams();

  PorousFlowAquiferBC(const InputParameters & parameters);

protected:
  virtual Real multiplier() const override;
  virtual Real dmultiplier_dvar(unsigned int pvar) const override;

private:
  /// Gravitational acceleration vector [m/s^2]
  const RealVectorValue _gravity;

  /// Magnitude of gravity
  const Real _g;

  /// True when aquifer_head formulation is used; false for pressure-at-datum formulation
  const bool _use_head_form;

  /// Far-field hydraulic head [m] (head formulation only)
  const Real _aquifer_head;

  /// Far-field pressure at datum_elevation [Pa] (pressure formulation only)
  const Real _p_datum;

  /// Reference elevation for pressure-at-datum formulation [m]
  const Real _datum_elevation;

  /// Conductance per unit boundary area [kg/(m^2 Pa s)] (head formulation only)
  const Real _conductance;

  /// Distance from boundary to far-field aquifer [m] (pressure formulation only)
  const Real _aquifer_distance;

  /// True when the user supplied aquifer_permeability; false means the boundary k_nn is used
  const bool _use_aquifer_perm;

  /// User-supplied permeability of the material between boundary and aquifer [m^2]
  const Real _aquifer_permeability;

  /// Nodal pore pressure in each phase [Pa]
  const MaterialProperty<std::vector<Real>> & _pp;

  /// d(nodal pore pressure)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dpp_dvar;

  /// Nodal fluid density for each phase [kg/m^3] -- used in the hydrostatic correction
  const MaterialProperty<std::vector<Real>> & _fluid_density_nodal;

  /// d(nodal fluid density)/d(PorousFlow variable) -- needed for the Jacobian
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_density_nodal_dvar;

  /// Permeability tensor at quadrature points (pressure formulation without
  /// aquifer_permeability only; null otherwise)
  const MaterialProperty<RealTensorValue> * const _permeability_qp;

  /// Nodal fluid viscosity for each phase (pressure formulation only; null for head form)
  const MaterialProperty<std::vector<Real>> * const _fluid_viscosity_nodal;

  /// d(nodal fluid viscosity)/d(PorousFlow variable) (pressure formulation only)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_viscosity_nodal_dvar;
};
