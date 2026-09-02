//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

class PorousFlowDictator;

/**
 * Robin (aquifer) boundary condition for finite-volume PorousFlow.
 *
 * FV analogue of PorousFlowAquiferBC.  Applies a flux proportional to the
 * difference between the boundary-cell pore pressure and a far-field aquifer
 * pressure that is corrected for elevation:
 *
 *   flux = massfrac * relperm * C * (P_cell - P_aquifer(z))
 *   [positive = fluid leaves domain]
 *
 * The aquifer reference pressure is computed from either:
 *  (a) a hydraulic head:  P_aq(z) = rho * g * (aquifer_head - z)
 *  (b) a pressure at datum: P_aq(z) = aquifer_pressure_at_datum
 *                                     + rho * g * (datum_elevation - z)
 * where rho is the boundary-cell fluid density and z is the elevation of the
 * boundary-cell centroid.  Comparing the cell pressure with the aquifer
 * pressure at the cell-centroid elevation makes the flux exactly zero in
 * hydrostatic equilibrium on boundaries of any orientation.
 *
 * Conductance specification:
 *  (a) head form:     user supplies aquifer_conductance [kg/(m^2 Pa s)]
 *  (b) pressure form: conductance is computed as rho * k_nn / (mu * L), where
 *      k_nn is the boundary-cell permeability normal to the boundary (or the
 *      optional aquifer_permeability parameter), mu is the fluid viscosity,
 *      and L = aquifer_distance is the distance to the far-field [m].
 */
class FVPorousFlowAquiferBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  FVPorousFlowAquiferBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// UserObject that holds information (number of phases, components, etc)
  const PorousFlowDictator & _dictator;
  /// Number of fluid phases present
  const unsigned int _num_phases;
  /// Index of the fluid phase this BC applies to
  const unsigned int _phase;
  /// Index of the fluid component this BC applies to
  const unsigned int _fluid_component;

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

  /// Fluid density
  const ADMaterialProperty<std::vector<Real>> & _density;
  const ADMaterialProperty<std::vector<Real>> & _density_neighbor;

  /// Fluid viscosity
  const ADMaterialProperty<std::vector<Real>> & _viscosity;
  const ADMaterialProperty<std::vector<Real>> & _viscosity_neighbor;

  /// Relative permeability
  const ADMaterialProperty<std::vector<Real>> & _relperm;
  const ADMaterialProperty<std::vector<Real>> & _relperm_neighbor;

  /// Mass fraction of fluid components in fluid phases
  const ADMaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions;
  const ADMaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions_neighbor;

  /// Permeability
  const ADMaterialProperty<RealTensorValue> & _permeability;
  const ADMaterialProperty<RealTensorValue> & _permeability_neighbor;

  /// Fluid pressure
  const ADMaterialProperty<std::vector<Real>> & _pressure;
  const ADMaterialProperty<std::vector<Real>> & _pressure_neighbor;
};
