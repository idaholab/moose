//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDBOUNDSVECTORSACTION_H
#define ADDBOUNDSVECTORSACTION_H

#include "Action.h"

class AddBoundsVectorsAction;

template <>
InputParameters validParams<AddBoundsVectorsAction>();

class AddBoundsVectorsAction : public Action
{
public:
  AddBoundsVectorsAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDBOUNDSVECTORSACTION_H
