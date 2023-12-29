//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DirichletBCBase.h"
#include "Function.h"

/**
 * This class applies a displacement time history on a given boundary in a given direction.
 * The displacement is converted to acceleration using backward euler differentiation and then
 * integrated using newmark time integration scheme to get the displacement. This modified
 * displacement is then applied to the boundary.
 **/

class PresetDisplacement : public DirichletBCBase
{
public:
  static InputParameters validParams();

  PresetDisplacement(const InputParameters & parameters);

protected:
  virtual Real computeQpValue();

  const VariableValue & _u_old;
  const Real _scale_factor;
  const Function & _function;
  const VariableValue & _vel_old;
  const VariableValue & _accel_old;
  const Real _beta;
};
