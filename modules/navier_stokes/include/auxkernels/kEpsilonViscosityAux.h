

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
 * Implements two near-wall treatments: equilibrium and non-equilibrium wall functions.
 */
class kEpsilonViscosityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  virtual void initialSetup() override;

  kEpsilonViscosityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Local method to find friction velocity.
  /// This method may need to be reimplemented for each new turbulence model
  ADReal findUStarLocalMethod(const ADReal & u, const Real & dist);

  /// The dimension of the domain
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
  const Real _C_mu;

  /// Wall boundaries
  const std::vector<BoundaryName> & _wall_boundary_names;

  /// If the user wants the linearized computation of y_plus
  const bool _linearized_yplus;

  /// If the user wants to enable bulk wall treatment
  const bool _bulk_wall_treatment;

  /// IF the user requested non-equilibrium wall treatment
  const bool _non_equilibrium_treatment;

  // -- Parameters of the wall function method

  /// Maximum number of iterations to find the friction velocity
  static constexpr int _MAX_ITERS_U_TAU{50};

  /// Relative tolerance to find the friction velocity
  static constexpr Real _REL_TOLERANCE{1e-4};

  ///@{
  /// Maps for wall bounded elements
  std::map<const Elem *, bool> _wall_bounded;
  std::map<const Elem *, std::vector<Real>> _dist;
  std::map<const Elem *, std::vector<const FaceInfo *>> _face_infos;
  ///@}
};
