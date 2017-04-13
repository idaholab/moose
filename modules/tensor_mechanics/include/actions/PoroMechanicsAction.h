/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POROMECHANICSACTION_H
#define POROMECHANICSACTION_H

#include "TensorMechanicsAction.h"

class PoroMechanicsAction;

template <>
InputParameters validParams<PoroMechanicsAction>();

class PoroMechanicsAction : public TensorMechanicsAction
{
public:
  PoroMechanicsAction(const InputParameters & params);

  virtual void act();
};

#endif // POROMECHANICSACTION_H
