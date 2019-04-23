//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

#include <string>

class SplitMeshAction;

template <>
InputParameters validParams<SplitMeshAction>();

class SplitMeshAction : public Action
{
public:
  SplitMeshAction(InputParameters params);

  virtual void act() override;
};

