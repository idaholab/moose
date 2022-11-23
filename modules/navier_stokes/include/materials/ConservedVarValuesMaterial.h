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
 * This object takes a conserved free-flow variable set (rho, rhoU, rho et) and computes all the
 * necessary quantities for solving the compressible free-flow Euler equations
 */
class ConservedVarValuesMaterial : public Material
{
public:
  ConservedVarValuesMaterial(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual void computeQpProperties() override;

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;

  const ADVariableValue & _var_rho;
  const ADVariableValue & _var_rho_u;
  const ADVariableValue & _var_rho_v;
  const ADVariableValue & _var_rho_w;
  const ADVariableValue & _var_total_energy_density;
  ADMaterialProperty<Real> & _rho;
  ADMaterialProperty<RealVectorValue> & _mass_flux;
  ADMaterialProperty<RealVectorValue> & _momentum;
  ADMaterialProperty<Real> & _total_energy_density;
  ADMaterialProperty<RealVectorValue> & _velocity;
  ADMaterialProperty<Real> & _speed;
  ADMaterialProperty<Real> & _vel_x;
  ADMaterialProperty<Real> & _vel_y;
  ADMaterialProperty<Real> & _vel_z;
  ADMaterialProperty<Real> & _rhou;
  ADMaterialProperty<Real> & _rhov;
  ADMaterialProperty<Real> & _rhow;
  ADMaterialProperty<Real> & _v;
  ADMaterialProperty<Real> & _specific_internal_energy;
  ADMaterialProperty<Real> & _pressure;
  ADMaterialProperty<Real> & _specific_total_enthalpy;
  ADMaterialProperty<Real> & _rho_ht;
  ADMaterialProperty<Real> & _T_fluid;
};
