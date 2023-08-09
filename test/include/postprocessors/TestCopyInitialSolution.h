//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * A postprocessor for testing initial solution equality (see #1396)
 */
class TestCopyInitialSolution : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TestCopyInitialSolution(const InputParameters & parameters);
  virtual ~TestCopyInitialSolution();
  virtual void initialize() override;
  virtual void execute() override;
  using Postprocessor::getValue;
  virtual Real getValue() const override;

protected:
  bool _value;
};
