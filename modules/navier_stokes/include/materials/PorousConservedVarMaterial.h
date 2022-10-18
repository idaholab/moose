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
 * This object takes a conserved porous-flow variable set (rho epsilon, rho epsilon U,
 * rho epsilon et) and computes all the necessary quantities for solving the compressible porous
 * Euler equations
 */
class PorousConservedVarMaterial : public Material
{
public:
  PorousConservedVarMaterial(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual void computeQpProperties() override;

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;

  const ADVariableValue & _var_rho;
  const ADVariableGradient & _var_grad_rho;
  const ADVariableValue & _var_rho_ud;
  const ADVariableValue & _var_rho_vd;
  const ADVariableValue & _var_rho_wd;
  const ADVariableGradient & _var_grad_rho_ud;
  const ADVariableGradient & _var_grad_rho_vd;
  const ADVariableGradient & _var_grad_rho_wd;
  const ADVariableValue & _var_total_energy_density;
  const ADVariableGradient & _var_grad_rho_et;
  const MaterialProperty<Real> & _epsilon;
  ADMaterialProperty<Real> & _rho;
  ADMaterialProperty<Real> & _superficial_rho;
  ADMaterialProperty<RealVectorValue> & _mass_flux;
  ADMaterialProperty<RealVectorValue> & _momentum;
  ADMaterialProperty<Real> & _total_energy_density;
  ADMaterialProperty<RealVectorValue> & _velocity;
  ADMaterialProperty<Real> & _speed;
  ADMaterialProperty<RealVectorValue> & _superficial_velocity;
  ADMaterialProperty<Real> & _sup_vel_x;
  ADMaterialProperty<Real> & _sup_vel_y;
  ADMaterialProperty<Real> & _sup_vel_z;
  ADMaterialProperty<RealVectorValue> & _grad_sup_vel_x;
  ADMaterialProperty<RealVectorValue> & _grad_sup_vel_y;
  ADMaterialProperty<RealVectorValue> & _grad_sup_vel_z;
  ADMaterialProperty<Real> & _sup_mom_x;
  ADMaterialProperty<Real> & _sup_mom_y;
  ADMaterialProperty<Real> & _sup_mom_z;
  ADMaterialProperty<Real> & _vel_x;
  ADMaterialProperty<Real> & _vel_y;
  ADMaterialProperty<Real> & _vel_z;
  ADMaterialProperty<Real> & _rhou;
  ADMaterialProperty<Real> & _rhov;
  ADMaterialProperty<Real> & _rhow;
  ADMaterialProperty<Real> & _v;
  ADMaterialProperty<Real> & _specific_internal_energy;
  ADMaterialProperty<Real> & _pressure;
  ADMaterialProperty<RealVectorValue> & _grad_pressure;
  ADMaterialProperty<Real> & _specific_total_enthalpy;
  ADMaterialProperty<Real> & _rho_ht;
  ADMaterialProperty<Real> & _superficial_rho_et;
  ADMaterialProperty<Real> & _superficial_rho_ht;
  ADMaterialProperty<Real> & _T_fluid;
  ADMaterialProperty<RealVectorValue> & _grad_T_fluid;
};
