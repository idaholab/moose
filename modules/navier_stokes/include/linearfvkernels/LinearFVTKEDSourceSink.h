//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVElementalKernel.h"

/**
 * Kernel that adds contributions to the source and the sink of the
 * turbulent kinetic energy dissipation
 * discretized using the finite volume method to a linear system.
 */
class LinearFVTKEDSourceSink : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVTKEDSourceSink(const InputParameters & params);

  virtual void initialSetup() override;

  virtual Real computeMatrixContribution() override;

  virtual Real computeRightHandSideContribution() override;

protected:
  /// The dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const Moose::Functor<Real> & _u_var;
  /// y-velocity
  const Moose::Functor<Real> * _v_var;
  /// z-velocity
  const Moose::Functor<Real> * _w_var;

  /// Turbulent kinetic energy
  const Moose::Functor<Real> & _k;

  /// Density
  const Moose::Functor<Real> & _rho;

  /// Dynamic viscosity
  const Moose::Functor<Real> & _mu;

  /// Turbulent dynamic viscosity
  const Moose::Functor<Real> & _mu_t;

  /// Wall boundaries
  const std::vector<BoundaryName> & _wall_boundary_names;

  /// If the user wants to use the linearized model
  const bool _linearized_model;

  /// Method used for wall treatment
  NS::WallTreatmentEnum _wall_treatment;

  /// Value of the first epsilon closure coefficient
  const Real _C1_eps;

  /// Value of the second epsilon closure coefficient
  const Real _C2_eps;

  /// C_mu constant
  const Real _C_mu;

  // Production Limiter Constant
  const Real _C_pl;

  ///@{
  /** Maps for wall treatment */
  std::map<const Elem *, bool> _wall_bounded;
  std::map<const Elem *, std::vector<Real>> _dist;
  std::map<const Elem *, std::vector<const FaceInfo *>> _face_infos;
  ///@}
};
