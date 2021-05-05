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

class PorousPrimitiveVarMaterial : public Material
{
public:
  PorousPrimitiveVarMaterial(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual void computeQpProperties() override;
  ADReal computeSpeed() const;

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;

  const ADVariableValue & _var_pressure;
  const ADVariableValue & _var_ud;
  const ADVariableValue & _var_vd;
  const ADVariableValue & _var_wd;
  const ADVariableValue & _var_T_fluid;
  const MaterialProperty<Real> & _epsilon;
  ADMaterialProperty<Real> & _rho;
  ADMaterialProperty<RealVectorValue> & _momentum;
  ADMaterialProperty<RealVectorValue> & _superficial_velocity;
  ADMaterialProperty<Real> & _vel_x;
  ADMaterialProperty<Real> & _vel_y;
  ADMaterialProperty<Real> & _vel_z;
  ADMaterialProperty<Real> & _mom_x;
  ADMaterialProperty<Real> & _mom_y;
  ADMaterialProperty<Real> & _mom_z;
  ADMaterialProperty<Real> & _specific_internal_energy;
  ADMaterialProperty<Real> & _pressure;
  ADMaterialProperty<Real> & _rho_ht;
  ADMaterialProperty<Real> & _T_fluid;
};
