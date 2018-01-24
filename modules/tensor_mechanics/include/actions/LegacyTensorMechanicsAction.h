//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
