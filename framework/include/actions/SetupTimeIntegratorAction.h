//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SETUPTIMEINTEGRATORACTION_H
#define SETUPTIMEINTEGRATORACTION_H

#include "MooseObjectAction.h"

class SetupTimeIntegratorAction;

template <>
InputParameters validParams<SetupTimeIntegratorAction>();

/**
 *
 */
class SetupTimeIntegratorAction : public MooseObjectAction
{
public:
  SetupTimeIntegratorAction(const InputParameters & parameters);

  virtual void act() override;
};

#endif /* SETUPTIMEINTEGRATORACTION_H */
