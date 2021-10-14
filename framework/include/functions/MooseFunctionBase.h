//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "SetupInterface.h"

class InputParameters;

class MooseFunctionBase : public MooseObject, public SetupInterface
{
public:
  MooseFunctionBase(const InputParameters & params);
  virtual ~MooseFunctionBase() = default;

  static InputParameters validParams();
};
