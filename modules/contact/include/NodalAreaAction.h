//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALAREAACTION_H
#define NODALAREAACTION_H

#include "MooseObjectAction.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class NodalAreaAction : public MooseObjectAction
{
public:
  NodalAreaAction(const InputParameters & params);

  virtual void act();
};

template <>
InputParameters validParams<NodalAreaAction>();

#endif
