/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
