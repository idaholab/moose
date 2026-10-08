//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * Test user object that holds a controllable real parameter
 */
class RealValueTestUserObject : public GeneralUserObject
{
public:
  RealValueTestUserObject(const InputParameters & params);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

public:
  static InputParameters validParams();
};
