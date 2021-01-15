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

class INSFVMaterial : public Material
{
public:
  static InputParameters validParams();

  INSFVMaterial(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  /// x-component velocity
  const ADVariableValue & _u_vel;

  /// y-component velocity
  const ADVariableValue & _v_vel;

  /// z-component velocity
  const ADVariableValue & _w_vel;

  /// pressure variable
  const ADVariableValue & _p_var;

  /// The velocity as a vector
  ADMaterialProperty<RealVectorValue> & _velocity;

  /// The density times the x-velocity
  ADMaterialProperty<Real> & _rho_u;

  /// The density times the y-velocity
  ADMaterialProperty<Real> & _rho_v;

  /// The density times the z-velocity
  ADMaterialProperty<Real> & _rho_w;

  /// The pressure material property
  ADMaterialProperty<Real> & _p;

  /// density
  const Real & _rho;

  const bool _has_temperature;

  const ADVariableValue * const _temperature;
  const ADMaterialProperty<Real> * const _cp;
  ADMaterialProperty<Real> * const _rho_cp_temp;
};
