//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SETADAPTIVITYOPTIONSACTION_H
#define SETADAPTIVITYOPTIONSACTION_H

#include "Action.h"

class SetAdaptivityOptionsAction;

template <>
InputParameters validParams<SetAdaptivityOptionsAction>();

class SetAdaptivityOptionsAction : public Action
{
public:
  SetAdaptivityOptionsAction(InputParameters params);

  virtual void act() override;
};

#endif // SETADAPTIVITYOPTIONSACTION_H
