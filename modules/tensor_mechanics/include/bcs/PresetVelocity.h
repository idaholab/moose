//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PRESETVELOCITY_H
#define PRESETVELOCITY_H

#include "PresetNodalBC.h"

class PresetVelocity : public PresetNodalBC
{
public:
  PresetVelocity(const InputParameters & parameters);

protected:
  virtual Real computeQpValue();

  const VariableValue & _u_old;
  const Real _velocity;
  Function & _function;
};

template <>
InputParameters validParams<PresetVelocity>();

#endif /* PRESETVELOCITY_H */
