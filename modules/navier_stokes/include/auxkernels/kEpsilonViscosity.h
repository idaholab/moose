//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

#include "INSFVVelocityVariable.h"
/**
 * Computes the turbuent viscosity for the k-Epsilon model.
 * Implements two near-wall treatements: equilibrium and non-equilibrium wall functions.
 */
class kEpsilonViscosity : public AuxKernel
{
public:
  static InputParameters validParams();

  kEpsilonViscosity(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// the dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const INSFVVelocityVariable * const _u_var;
  /// y-velocity
  const INSFVVelocityVariable * const _v_var;
  /// z-velocity
  const INSFVVelocityVariable * const _w_var;

  /// Turbulent kinetic energy
  const Moose::Functor<ADReal> & _k;
  /// Turbulent kinetic energy dissipation rate
  const Moose::Functor<ADReal> & _epsilon;

  /// Density
  const Moose::Functor<ADReal> & _rho;
  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;
  /// C-mu closure coefficient
  const Moose::Functor<ADReal> & _C_mu;
  /// Wall boundaries
  std::vector<BoundaryName> _wall_boundary_names;

  /// Linearzied computation of y_plus ?
  const bool _linearized_yplus;

  /// Number of auxiliary kernel iterations
  unsigned int _n_kernel_iters;

  /// Relxation iteration for activating the k-epsilon model
  const Real _n_iters_activate;

  /// Maximum mixing length allowed for the domain
  const Real _max_mixing_length;

  /// Linearzied computation of y_plus ?
  const bool _wall_treatement;

  /// Non-equilibrium wall treatement ?
  const bool _non_equilibrium_treatement;
};
