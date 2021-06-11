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
#include "PorousFlowDictator.h"

/**
 * Applies a flux to a boundary such that fluid or heat will flow freely out of the boundary.
 */
class PorousFlowOutflowBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  PorousFlowOutflowBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Number of phases in the simulation
  const unsigned _num_phases;

  /// Flag indicating whether to include derivatives of the permeability in the Jacobian
  const bool _perm_derivs;

  /// Indicates the type of flux that this BC will apply
  const enum class FluxTypeChoiceEnum { FLUID, HEAT } _flux_type;

  /// The fluid component number (only used if _flux_type==FLUID))
  const unsigned int _sp;

  /// Whether to multiply the Darcy flux by the fluid density.  This is automatically set to true if _flux_type==HEAT
  const bool _multiply_by_density;

  /// Whether to multiply the Darcy flux by the relative permeability
  const bool _include_relperm;

  /// Gravitational acceleration
  const RealVectorValue _gravity;

  /// Multiply the flux by this quantity.  This is mainly used for testing purposes, for instance, testing the Jacobian, and should ordinarily set to its default value of 1.0
  const Real _multiplier;

  /// Gradient of the pore pressure in each phase
  const MaterialProperty<std::vector<RealGradient>> & _grad_p;

  /// Derivative of Grad porepressure in each phase wrt grad(PorousFlow variables)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dgrad_p_dgrad_var;

  /// Derivative of Grad porepressure in each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<RealGradient>>> & _dgrad_p_dvar;

  /// Fluid density for each phase (at the qp)
  const MaterialProperty<std::vector<Real>> & _fluid_density_qp;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables (at the qp)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_density_qp_dvar;

  /// Permeability of porous material
  const MaterialProperty<RealTensorValue> & _permeability;

  /// d(permeabiity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealTensorValue>> & _dpermeability_dvar;

  /// d(permeabiity)/d(grad(PorousFlow variable))
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _dpermeability_dgradvar;

  /// Viscosity of each phase
  const MaterialProperty<std::vector<Real>> & _fluid_viscosity;

  /// Derivative of the fluid viscosity for each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_viscosity_dvar;

  /// Whether there is a fluid_phase_density_nodal Material
  const bool _has_density;

  /// Whether there is a mass_frac_nodal Material
  const bool _has_mass_fraction;

  /// Whether there is a relative_permeability_nodal Material
  const bool _has_relperm;

  /// Whether there is an enthalpy Material
  const bool _has_enthalpy;

  /// Whether there is a thermal_conductivity_qp Material
  const bool _has_thermal_conductivity;

  /// Whether there is a grad_temp Material
  const bool _has_t;

  /// Fluid density for each phase (at the node)
  const MaterialProperty<std::vector<Real>> * const _fluid_density_node;

  /// d(Fluid density for each phase (at the node))/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_node_dvar;

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

  /// Thermal_Conductivity of porous material
  const MaterialProperty<RealTensorValue> * const _thermal_conductivity;

  /// d(Thermal_Conductivity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealTensorValue>> * const _dthermal_conductivity_dvar;

  /// grad(temperature)
  const MaterialProperty<RealGradient> * const _grad_t;

  /// d(gradT)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> * const _dgrad_t_dvar;

  /// d(gradT)/d(grad PorousFlow variable)
  const MaterialProperty<std::vector<Real>> * const _dgrad_t_dgradvar;

private:
  /// Derivative of residual with respect to the jvar variable
  Real jac(unsigned int jvar) const;

  /// Darcy contribution to the outflowBC
  Real darcy(unsigned ph) const;

  /// Mobility contribution to the outflowBC
  Real mobility(unsigned ph) const;

  /// Either mass_fraction or enthalpy, depending on _flux_type
  Real prefactor(unsigned ph) const;
};
