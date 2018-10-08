//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SETUPDEBUGACTION_H
#define SETUPDEBUGACTION_H

// MOOSE includes
#include "Action.h"

class SetupDebugAction;
class MooseObjectAction;

template <>
InputParameters validParams<SetupDebugAction>();

class SetupDebugAction : public Action
{
public:
  SetupDebugAction(InputParameters parameters);

  virtual void act() override;
};

#endif /* SETUPDEBUGACTION_H */
