//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SETUPMESHCOMPLETEACTION_H
#define SETUPMESHCOMPLETEACTION_H

#include "Action.h"

class SetupMeshCompleteAction;

template <>
InputParameters validParams<SetupMeshCompleteAction>();

class SetupMeshCompleteAction : public Action
{
public:
  SetupMeshCompleteAction(InputParameters params);

  bool completeSetup(MooseMesh * mesh);

  virtual void act() override;
};

#endif // SETUPMESHCOMPLETEACTION_H
