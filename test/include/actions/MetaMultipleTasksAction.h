//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef METAMULTIPLETASKSACTION_H
#define METAMULTIPLETASKSACTION_H

#include "Action.h"

class MetaMultipleTasksAction : public Action
{
public:
  MetaMultipleTasksAction(const InputParameters & params);

  virtual void act();
};

template <>
InputParameters validParams<MetaMultipleTasksAction>();

#endif // METAMULTIPLETASKSACTION_H
