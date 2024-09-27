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
#include "MathFVUtils.h"
#include "INSFVMomentumResidualObject.h"
#include "INSFVVelocityVariable.h"
#include "NS.h"

/**
 * Computes the source and sink terms for the turbulent kinetic energy dissipation rate.
 */
class INSFVTKEDSourceSink : public FVElementalKernel
{
public:
  static InputParameters validParams();

  virtual void initialSetup() override;

  INSFVTKEDSourceSink(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

protected:
  /// The dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const Moose::Functor<ADReal> & _u_var;
  /// y-velocity
  const Moose::Functor<ADReal> * _v_var;
  /// z-velocity
  const Moose::Functor<ADReal> * _w_var;

  /// Turbulent kinetic energy
  const Moose::Functor<ADReal> & _k;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// Turbulent dynamic viscosity
  const Moose::Functor<ADReal> & _mu_t;

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

  /// Stored strain rate
  std::map<const Elem *, Real> _symmetric_strain_tensor_norm_old;
  /// Map for the previous destruction field
  std::map<const Elem *, Real> _old_destruction;

  /// Map for the previous nonlienar iterate
  std::map<const Elem *, Real> _pevious_nl_sol;

  ///@{
  /** Maps for wall treatment */
  std::map<const Elem *, bool> _wall_bounded;
  std::map<const Elem *, std::vector<Real>> _dist;
  std::map<const Elem *, std::vector<const FaceInfo *>> _face_infos;
  ///@}

  /// Whether a nonlinear Newton-like solver is being used (as opposed to a linearized solver)
  const bool _newton_solve;
};
