//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

class SinglePhaseFluidProperties;

/**
 * This object takes a mixed porous-flow variable set (pressure, rho epsilon U, T_fluid) and
 * computes all the necessary quantities for solving the compressible porous Euler equations
 */
class PorousMixedVarMaterial : public Material
{
public:
  PorousMixedVarMaterial(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual void computeQpProperties() override;

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;

  /// primitive variables
  const ADVariableValue & _var_pressure;
  const ADVariableGradient & _grad_var_pressure;
  const ADVariableValue & _pressure_dot;
  const ADVariableValue & _var_T_fluid;
  const ADVariableGradient & _grad_var_T_fluid;
  const ADVariableValue & _T_fluid_dot;

  const ADVariableValue & _var_sup_mom_x;
  const ADVariableGradient & _grad_var_sup_mom_x;
  const ADVariableValue & _var_sup_mom_y;
  const ADVariableGradient & _grad_var_sup_mom_y;
  const ADVariableValue & _var_sup_mom_z;
  const ADVariableGradient & _grad_var_sup_mom_z;
  const ADVariableValue & _var_sup_mom_x_dot;
  const ADVariableValue & _var_sup_mom_y_dot;
  const ADVariableValue & _var_sup_mom_z_dot;

  /// porosity
  const MaterialProperty<Real> & _epsilon;

  /// properties: primitives
  ADMaterialProperty<Real> & _pressure;
  ADMaterialProperty<RealVectorValue> & _grad_pressure;
  ADMaterialProperty<Real> & _T_fluid;
  ADMaterialProperty<RealVectorValue> & _grad_T_fluid;
  ADMaterialProperty<RealVectorValue> & _sup_vel;
  ADMaterialProperty<Real> & _sup_vel_x;
  ADMaterialProperty<RealVectorValue> & _grad_sup_vel_x;
  ADMaterialProperty<Real> & _sup_vel_y;
  ADMaterialProperty<RealVectorValue> & _grad_sup_vel_y;
  ADMaterialProperty<Real> & _sup_vel_z;
  ADMaterialProperty<RealVectorValue> & _grad_sup_vel_z;

  ADMaterialProperty<Real> & _rho;
  ADMaterialProperty<Real> & _sup_rho_dot;
  ADMaterialProperty<RealVectorValue> & _velocity;
  ADMaterialProperty<Real> & _vel_x;
  ADMaterialProperty<Real> & _vel_y;
  ADMaterialProperty<Real> & _vel_z;
  ADMaterialProperty<Real> & _sup_mom_x;
  ADMaterialProperty<Real> & _sup_mom_y;
  ADMaterialProperty<Real> & _sup_mom_z;
  ADMaterialProperty<Real> & _sup_mom_x_dot;
  ADMaterialProperty<Real> & _sup_mom_y_dot;
  ADMaterialProperty<Real> & _sup_mom_z_dot;
  ADMaterialProperty<Real> & _sup_rho_et_dot;
  ADMaterialProperty<RealVectorValue> & _mom;
  ADMaterialProperty<Real> & _mom_x;
  ADMaterialProperty<Real> & _mom_y;
  ADMaterialProperty<Real> & _mom_z;
  ADMaterialProperty<Real> & _speed;
  ADMaterialProperty<Real> & _rho_et;
  ADMaterialProperty<Real> & _e;
  ADMaterialProperty<Real> & _ht;
};
