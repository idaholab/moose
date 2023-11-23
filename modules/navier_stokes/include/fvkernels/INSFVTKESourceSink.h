//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

/**
 * Simple class to demonstrate off diagonal Jacobian contributions.
 */
class INSFVTKESourceSink : public FVElementalKernel
{
public:
  static InputParameters validParams();

  virtual void initialSetup() override;

  INSFVTKESourceSink(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

protected:
  /// The dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const INSFVVelocityVariable * const _u_var;
  /// y-velocity
  const INSFVVelocityVariable * const _v_var;
  /// z-velocity
  const INSFVVelocityVariable * const _w_var;

  /// epsilon - dissipation rate of TKE
  const Moose::Functor<ADReal> & _epsilon;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// Turbulent dynamic viscosity
  const Moose::Functor<ADReal> & _mu_t;

  /// Wall boundaries
  std::vector<BoundaryName> _wall_boundary_names;

  /// Maximum mixing length allowed for the domain
  const Real _max_mixing_length;

  /// Linearized model?
  const bool _linearized_model;

  /// No equilibrium treatement
  const bool _non_equilibrium_treatement;

  /// C_mu constant
  const Real _C_mu;

  /// Maps for wall treatement
  std::map<const Elem *, bool> _wall_bounded;
  std::map<const Elem *, std::vector<Real>> _dist;
  std::map<const Elem *, std::vector<Point>> _normal;
  std::map<const Elem *, std::vector<const FaceInfo *>> _face_infos;

  /// -- Constants of the method
  static constexpr Real _von_karman{0.4187};
  static constexpr Real E{9.793};
};
