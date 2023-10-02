//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * UserObject for testing a restartable value late
 */
class LateRestartLoadTest : public GeneralUserObject
{
public:
  static InputParameters validParams();

  LateRestartLoadTest(const InputParameters & params);

  virtual void initialSetup() override;

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

private:
  const MooseEnum _test_method;
  const unsigned int _value;
  const unsigned int * const _restartable_value;
};
