//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

class Function;

/**
 * MaterialVectorBodyForce applies a body force (force/volume) given as a vector material property
 */
class MaterialVectorBodyForce : public Kernel
{
public:
  static InputParameters validParams();

  MaterialVectorBodyForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  /// coordinate axis this Kernel acts on
  unsigned int _component;

  /// coupled body force vector property
  const MaterialProperty<RealVectorValue> & _body_force;

  /// optional scaling function
  const Function & _function;

  // alpha parameter for HHT time integration scheme
  const Real _alpha;
};
