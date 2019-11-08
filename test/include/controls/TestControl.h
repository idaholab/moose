//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Control.h"

/**
 * A Control object for testing purposes
 */
class TestControl : public Control
{
public:
  static InputParameters validParams();

  TestControl(const InputParameters & parameters);
  virtual ~TestControl(){};
  virtual void execute();

private:
  /// The type of test to perform
  MooseEnum _test_type;

  /// Storage for the alias test
  const MooseObjectParameterName _alias;
};
