//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariablePostprocessor.h"

/**
 * Test user object that outputs on initialization and finalize to check if
 *  multiple loops over the mesh were taken to test the execution_order_group system.
 */
class ExecutionGroupTestPostprocessor : public ElementIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  ExecutionGroupTestPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
};
