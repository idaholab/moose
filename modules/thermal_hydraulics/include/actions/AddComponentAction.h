//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectAction.h"

class AddComponentAction : public MooseObjectAction
{
public:
  AddComponentAction(const InputParameters & params);

  virtual void act() override;

protected:
  /// True if building a component group
  bool _group;

public:
  static InputParameters validParams();
};
