//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CREATEPROBLEMDEFAULTACTION_H
#define CREATEPROBLEMDEFAULTACTION_H

// MOOSE includes
#include "MooseObjectAction.h"
#include "MultiMooseEnum.h"

class CreateProblemDefaultAction;

template <>
InputParameters validParams<CreateProblemDefaultAction>();

class CreateProblemDefaultAction : public MooseObjectAction
{
public:
  CreateProblemDefaultAction(InputParameters parameters);

  virtual void act() override;
};

#endif /* CREATEPROBLEMDEFAULTACTION_H */
