

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
class kEpsilonViscosityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  kEpsilonViscosityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
  virtual void initialSetup() override;
  // virtual void meshChanged() override;

  // Local method to find friction velocty
  // Note: this method may be to need reimplemented for each new turbulent model
  ADReal findUStarLocalMethod(const ADReal & u, const Real & dist);

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

  /// Relaxation Factor
  const Real _rf;

  /// -- Parameters of the wall function method

  /// Maximum number of iterations to find the friction velocity
  static constexpr int _MAX_ITERS_U_TAU{50};

  /// Relative tolerance to find the friction velocity
  static constexpr Real _REL_TOLERANCE{1e-6};

  /// -- Constants of the method

  static constexpr Real _von_karman{0.4187};
  static constexpr Real _E{9.793};

  /// Viscosity limiter - maximum viscosity must be at the wall
  Real _max_viscosity_value;

  /// -- Number of iterations needed to activate the computation of mu_t
  unsigned int _iters_to_activate;

  /// -- Initial value for mu_t
  Real _mu_t_inital;

  /// -- Relaxation method for production and destruction
  const MooseEnum _relaxation_method;

  /// -- Element Localized Damping
  std::map<const Elem *, Real> _nl_damping_map;

  /// -- Damping values for nonlinear iterations
  Real _damper;
};
