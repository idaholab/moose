//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PARTITIONERACTION_H
#define PARTITIONERACTION_H

#include "MooseObjectAction.h"

class PartitionerAction;

template <>
InputParameters validParams<PartitionerAction>();

class PartitionerAction : public MooseObjectAction
{
public:
  PartitionerAction(InputParameters params);

  virtual void act() override;
};

#endif // PartitionerAction_H
