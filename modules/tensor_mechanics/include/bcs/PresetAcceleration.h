//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PRESETACCELERATION_H
#define PRESETACCELERATION_H

#include "PresetNodalBC.h"

/**
 * This class prescribes the acceleration on a given boundary in a given direction.
 * The acceleration is integrated using newmark time integration scheme and the resulting
 * displacement is applied on the boundary.
 **/

class PresetAcceleration : public PresetNodalBC
{
public:
  PresetAcceleration(const InputParameters & parameters);

protected:
  virtual Real computeQpValue();

  const VariableValue & _u_old;
  const Real _scale_factor;
  Function & _function;
  const VariableValue & _vel_old;
  const VariableValue & _accel_old;
  const Real _beta;
};

template <>
InputParameters validParams<PresetAcceleration>();

#endif /* PRESETACCERELATION_H */
