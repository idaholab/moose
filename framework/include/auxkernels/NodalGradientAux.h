/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NodalGradientAux_H
#define NodalGradientAux_H

#include "NodalPatchRecovery.h"

class NodalGradientAux;

template <>
InputParameters validParams<NodalGradientAux>();

class NodalGradientAux : public NodalPatchRecovery
{
public:
  NodalGradientAux(const InputParameters & parameters);
  virtual ~NodalGradientAux() {}

protected:
  virtual Real computeValue() override;

  const unsigned _component;
  const VariableGradient & _grad;
};

#endif // NodalGradientAux_H
