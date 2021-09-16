//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

class INSFVMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  INSFVMaterial(const InputParameters & parameters);

protected:
  /// x-component velocity
  const MooseVariableFVReal & _u_vel;

  /// y-component velocity
  const MooseVariableFVReal * const _v_vel;

  /// z-component velocity
  const MooseVariableFVReal * const _w_vel;

  /// pressure variable
  const MooseVariableFVReal & _p_var;

  /// The velocity as a vector
  FunctorMaterialProperty<ADRealVectorValue> & _velocity;

  /// The density times the x-velocity
  FunctorMaterialProperty<ADReal> & _rho_u;

  /// The density times the y-velocity
  FunctorMaterialProperty<ADReal> & _rho_v;

  /// The density times the z-velocity
  FunctorMaterialProperty<ADReal> & _rho_w;

  /// The pressure material property
  FunctorMaterialProperty<ADReal> & _p;

  /// density
  const Moose::Functor<ADReal> & _rho;

  const bool _has_temperature;

  const MooseVariableFVReal * const _temperature;
  const Moose::Functor<ADReal> * const _cp;
  FunctorMaterialProperty<ADReal> * const _rho_cp_temp;
};
