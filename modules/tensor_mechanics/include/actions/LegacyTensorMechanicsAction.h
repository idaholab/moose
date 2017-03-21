/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LEGACYTENSORMECHANICSACTION_H
#define LEGACYTENSORMECHANICSACTION_H

#include "TensorMechanicsAction.h"

class LegacyTensorMechanicsAction;

template <>
InputParameters validParams<LegacyTensorMechanicsAction>();

class LegacyTensorMechanicsAction : public TensorMechanicsAction
{
public:
  LegacyTensorMechanicsAction(const InputParameters & params);

  virtual void act();
};

#endif // LEGACYTENSORMECHANICSACTION_H
