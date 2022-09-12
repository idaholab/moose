//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TestAction.h"

/**
 * Base class for adding common actions for Jacobian tests
 */
class JacobianTestAction : public TestAction
{
public:
  JacobianTestAction(const InputParameters & params);

protected:
  virtual void addPreconditioner() override;

  /// Finite differencing parameter
  const std::string _snes_test_err;

public:
  static InputParameters validParams();
};
