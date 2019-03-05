//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADMATERIAL_H
#define INSADMATERIAL_H

#include "ADMaterial.h"

template <ComputeStage>
class INSADMaterial;

declareADValidParams(INSADMaterial);

template <ComputeStage compute_stage>
class INSADMaterial : public ADMaterial<compute_stage>
{
public:
  INSADMaterial(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;
  virtual void computeQpProperties() override;

  /// velocity
  const ADVectorVariableValue & _velocity;

  /// gradient of velocity
  const ADVectorVariableGradient & _grad_velocity;

  /// gradient of the pressure
  const ADVariableGradient & _grad_p;

  /// viscosity
  const ADMaterialProperty(Real) & _mu;

  /// density
  const MaterialProperty<Real> & _rho;

  /// Whether we are performing a transient or steady simulation
  const bool _transient_term;

  /// Time derivative of the velocity, e.g. the acceleration
  const ADVectorVariableValue * _velocity_dot;

  /// The form of the visocus term in the momentum equations. Options are traction or laplace (default)
  const MooseEnum _viscous_form;

  /// Whether to integrate the pressure term in the momentum equations by parts
  const bool _integrate_p_by_parts;

  /// Whether to include the strong form of the viscous term in the momentum equation strong residual.
  /// The method is more consistent if set to true, but it incurs quite a bit more computational
  /// expense. Note that at present we don't actually support the `true` value of this member!
  const bool _include_viscous_term_in_strong_form;

  /// Whether the user set a gravity vector. If none is set, we assume there is no gravity term in the simulation
  bool _gravity_set;

  /// The gravity vector; should be in units of acceleration
  RealVectorValue _gravity;

  /// The strong residual of the mass continuity equation
  ADMaterialProperty(Real) & _mass_strong_residual;

  /// Strong residual corresponding to the momentum convective term
  ADMaterialProperty(RealVectorValue) & _convective_strong_residual;

  /// Strong residual corresponding to the momentum transient term
  ADMaterialProperty(RealVectorValue) & _td_strong_residual;

  /// Strong residual corresponding to the momentum gravity term
  MaterialProperty<RealVectorValue> & _gravity_strong_residual;

  /// Strong residual corresponding to the mms function term
  MaterialProperty<RealVectorValue> & _mms_function_strong_residual;

  /// The strong residual of the momentum equation
  ADMaterialProperty(Real) & _momentum_strong_residual;

  /// The x velocity mms forcing function
  Function & _x_vel_fn;

  /// The y velocity mms forcing function
  Function & _y_vel_fn;

  /// The z velocity mms forcing function
  Function & _z_vel_fn;
};

#endif // INSADMATERIAL_H
