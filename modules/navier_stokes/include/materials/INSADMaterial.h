//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"

class INSADMaterial : public ADMaterial
{
public:
  static InputParameters validParams();

  INSADMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// velocity
  const ADVectorVariableValue & _velocity;

  /// gradient of velocity
  const ADVectorVariableGradient & _grad_velocity;

  /// gradient of the pressure
  const ADVariableGradient & _grad_p;

  /// viscosity
  const ADMaterialProperty<Real> & _mu;

  /// density
  const ADMaterialProperty<Real> & _rho;

  /// Whether we are performing a transient or steady simulation
  const bool _transient_term;

  /// Time derivative of the velocity, e.g. the acceleration
  const ADVectorVariableValue * _velocity_dot;

  /// Whether to integrate the pressure term in the momentum equations by parts
  const bool _integrate_p_by_parts;

  /// Whether the user set a gravity vector. If none is set, we assume there is no gravity term in the simulation
  bool _gravity_set;

  /// The gravity vector; should be in units of acceleration
  RealVectorValue _gravity;

  /// The strong residual of the mass continuity equation
  ADMaterialProperty<Real> & _mass_strong_residual;

  /// Strong residual corresponding to the momentum convective term
  ADMaterialProperty<RealVectorValue> & _convective_strong_residual;

  /// Strong residual corresponding to the momentum viscous term. This is only used by stabilization
  /// kernels
  ADMaterialProperty<RealVectorValue> & _viscous_strong_residual;

  /// Strong residual corresponding to the momentum transient term
  ADMaterialProperty<RealVectorValue> & _td_strong_residual;

  /// Strong residual corresponding to the momentum gravity term
  ADMaterialProperty<RealVectorValue> & _gravity_strong_residual;

  /// Strong residual corresponding to the mms function term
  MaterialProperty<RealVectorValue> & _mms_function_strong_residual;

  /// The strong residual of the momentum equation
  ADMaterialProperty<RealVectorValue> & _momentum_strong_residual;

  /// The x velocity mms forcing function
  const Function & _x_vel_fn;

  /// The y velocity mms forcing function
  const Function & _y_vel_fn;

  /// The z velocity mms forcing function
  const Function & _z_vel_fn;

  /// Whether we are on the displaced mesh
  const bool _use_displaced_mesh;

  /// The quadrature points with potential partial derivatives with respect to displacement degrees
  /// of freedom
  const MooseArray<ADPoint> & _ad_q_point;
};
