//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "Function.h"
#include "PorousFlowDictator.h"

/**
 * Applies a flux sink to a boundary.
 * The strength of the flux is specified by flux_function.
 * In addition, this sink can be multiplied by:
 *  (1) the relative permeability of the fluid at the nodes
 *  (2) perm_nn*density/viscosity (the so-called mobility)
 *      where perm_nn is the permeability tensor projected
 *      to the normal direction.
 *  (3) the mass_fraction of a component at the nodes
 *  (4) the enthalpy of the phase
 *  (5) the internal energy of the phase
 *  (6) the thermal conductivity of the medium
 */
class PorousFlowSink : public IntegratedBC
{
public:
  static InputParameters validParams();

  PorousFlowSink(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Whether this BC involves fluid (whether the user has supplied a fluid phase number)
  const bool _involves_fluid;

  /// The phase number
  const unsigned int _ph;

  /// Whether the flux will be multiplied by the mass fraction
  const bool _use_mass_fraction;

  /// Whether there is a "mass_fraction" Material.  This is just for error checking
  const bool _has_mass_fraction;

  /// The component number (only used if _use_mass_fraction==true)
  const unsigned int _sp;

  /// Whether to multiply the sink flux by permeability*density/viscosity
  const bool _use_mobility;

  /// Whether there are Materials that can form "mobility".  This is just for error checking
  const bool _has_mobility;

  /// Whether to multiply the sink flux by relative permeability
  const bool _use_relperm;

  /// Whether there is a "relperm" Material.  This is just for error checking
  const bool _has_relperm;

  /// Whether to multiply the sink flux by enthalpy
  const bool _use_enthalpy;

  /// Whether there is an "enthalpy" Material.  This is just for error checking
  const bool _has_enthalpy;

  /// Whether to multiply the sink flux by internal_energy
  const bool _use_internal_energy;

  /// Whether there is an "internal_energy" Material.  This is just for error checking
  const bool _has_internal_energy;

  /// Whether to multiply the sink flux by thermal_conductivity
  const bool _use_thermal_conductivity;

  /// Whether there is an "thermal_conductivity" Material.  This is just for error checking
  const bool _has_thermal_conductivity;

  /// The flux
  const Function & _m_func;

  /// Permeability of porous material
  const MaterialProperty<RealTensorValue> * const _permeability;

  /// d(Permeability)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealTensorValue>> * const _dpermeability_dvar;

  /// d(Permeability)/d(grad(PorousFlow variable))
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> * const _dpermeability_dgradvar;

  /// Fluid density for each phase (at the node)
  const MaterialProperty<std::vector<Real>> * const _fluid_density_node;

  /// d(Fluid density for each phase (at the node))/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_node_dvar;

  /// Viscosity of each component in each phase
  const MaterialProperty<std::vector<Real>> * const _fluid_viscosity;

  /// d(Viscosity of each component in each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_viscosity_dvar;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real>> * const _relative_permeability;

  /// d(Relative permeability of each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _drelative_permeability_dvar;

  /// Mass fraction of each component in each phase
  const MaterialProperty<std::vector<std::vector<Real>>> * const _mass_fractions;

  /// d(Mass fraction of each component in each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> * const _dmass_fractions_dvar;

  /// Enthalpy of each phase
  const MaterialProperty<std::vector<Real>> * const _enthalpy;

  /// d(enthalpy of each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _denthalpy_dvar;

  /// Internal_Energy of each phase
  const MaterialProperty<std::vector<Real>> * const _internal_energy;

  /// d(internal_energy of each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dinternal_energy_dvar;

  /// Thermal_Conductivity of porous material
  const MaterialProperty<RealTensorValue> * const _thermal_conductivity;

  /// d(Thermal_Conductivity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealTensorValue>> * const _dthermal_conductivity_dvar;

  /// Derivative of residual with respect to the jvar variable
  Real jac(unsigned int jvar) const;

  /// The flux gets multiplied by this quantity
  virtual Real multiplier() const;

  /// d(multiplier)/d(Porous flow variable pvar)
  virtual Real dmultiplier_dvar(unsigned int pvar) const;

  /// Flag to check whether permeabiity derivatives are non-zero
  const bool _perm_derivs;
};
