//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CREATEDISPLACEDPROBLEMACTION_H
#define CREATEDISPLACEDPROBLEMACTION_H

#include "Action.h"

class CreateDisplacedProblemAction;

template <>
InputParameters validParams<CreateDisplacedProblemAction>();

/**
 *
 */
class CreateDisplacedProblemAction : public Action
{
public:
  CreateDisplacedProblemAction(InputParameters parameters);

  virtual void act() override;
};

#endif /* CREATEDISPLACEDPROBLEMACTION_H */
