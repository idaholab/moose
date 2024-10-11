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

/**
 * Computes the source and sink terms for the turbulent kinetic energy dissipation rate.
 */
class INSFVTKESDSourceSink : public FVElementalKernel
{
public:
  static InputParameters validParams();

  virtual void initialSetup() override;

  INSFVTKESDSourceSink(const InputParameters & parameters);

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
  const MooseEnum _wall_treatment;

  /// Blending functions
  const Moose::Functor<ADReal> & _F1;
  const Moose::Functor<ADReal> & _F2;

  /// Stored strain rate
  std::map<const Elem *, Real> _symmetric_strain_tensor_norm_old;
  /// Map for the previous destruction field
  std::map<const Elem *, Real> _old_destruction;

  /// Activate free-shear modification bool
  const bool _bool_free_shear_modficiation;

  // Wall normal unit vectors at each cell
  const Moose::Functor<ADRealVectorValue> * _wall_normal_unit_vectors;

  // Activate vortex stretching modification
  const bool _bool_vortex_stretching_modficiation;

  /// Activate free-shear modification bool
  const bool _bool_low_Re_modification;

  /// Map for the previous nonlienar iterate
  std::map<const Elem *, Real> _pevious_nl_sol;

  ///@{
  /** Maps for wall treatment */
  std::map<const Elem *, bool> _wall_bounded;
  std::map<const Elem *, std::vector<Real>> _dist;
  std::map<const Elem *, std::vector<const FaceInfo *>> _face_infos;
  ///@}

  /// Model constants
  static constexpr Real _C_mu = 0.09;
  static constexpr Real _beta_i_1 = 0.075;
  static constexpr Real _beta_i_2 = 0.0828;
  static constexpr Real _beta_infty = 0.09;
  static constexpr Real _alpha_0 = 1.0 / 9.0;
  static constexpr Real _sigma_omega_1 = 0.5;   // 2.000;
  static constexpr Real _sigma_omega_2 = 0.856; // 1.168;
  static constexpr Real _gamma_infty_1 = _beta_i_1 / _beta_infty - 0.1681 * _sigma_omega_1 / (0.3);
  static constexpr Real _gamma_infty_2 = _beta_i_2 / _beta_infty - 0.1681 * _sigma_omega_2 / (0.3);
  // Limiting
  static constexpr Real _c_pl = 10.0;
  static constexpr Real _a_1 = 0.31;
  // Low-Re specific
  static constexpr Real _Re_k = 6.0;
  static constexpr Real _Re_omega = 2.95;
  static constexpr Real _alpha_2_star = 1.0;
};
