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
 * Kernel that adds contributions to the source and the sink of the turbulent kinetic energy
 * discretized using the finite volume method to a linear system.
 */
class LinearFVTKESourceSink : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVTKESourceSink(const InputParameters & params);

  virtual void initialSetup() override;

  virtual Real computeMatrixContribution() override;

  virtual Real computeRightHandSideContribution() override;

protected:
  /// The dimension of the domain
  const unsigned int _dim;

  /// x-velocity
  const Moose::Functor<Real> & _u_var;
  /// y-velocity
  const Moose::Functor<Real> * _v_var;
  /// z-velocity
  const Moose::Functor<Real> * _w_var;

  /// epsilon - dissipation rate of TKE
  const Moose::Functor<Real> & _epsilon;

  /// Density
  const Moose::Functor<Real> & _rho;

  /// Dynamic viscosity
  const Moose::Functor<Real> & _mu;

  /// Turbulent dynamic viscosity
  const Moose::Functor<Real> & _mu_t;

  /// Wall boundaries
  const std::vector<BoundaryName> & _wall_boundary_names;

  /// Linearized model?
  const bool _linearized_model;

  /// Method used for wall treatment
  NS::WallTreatmentEnum _wall_treatment;

  /// C_mu constant
  const Real _C_mu;

  // Production Limiter Constant
  const Real _C_pl;

  ///@{
  /// Maps for wall treatement
  std::map<const Elem *, bool> _wall_bounded;
  std::map<const Elem *, std::vector<Real>> _dist;
  std::map<const Elem *, std::vector<const FaceInfo *>> _face_infos;
  ///@}
};
