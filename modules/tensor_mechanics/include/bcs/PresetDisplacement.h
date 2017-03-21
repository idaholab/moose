/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PRESETDISPLACEMENT_H
#define PRESETDISPLACEMENT_H

#include "PresetNodalBC.h"
#include "Function.h"

/**
 * This class applies a displacement time history on a given boundary in a given direction.
 * The displacement is converted to acceleration using backward euler differentiation and then
 * integrated using newmark time integration scheme to get the displacement. This modified
 * displacement is then applied to the boundary.
 **/

class PresetDisplacement : public PresetNodalBC
{
public:
  PresetDisplacement(const InputParameters & parameters);

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
InputParameters validParams<PresetDisplacement>();

#endif /* PRESETDISPLACEMENT_H */
