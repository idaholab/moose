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
 * Writes to _console and errors if the message is still buffered in the stream afterwards, which
 * happens when no Console output exists and the buffer is never cleared.
 */
class ConsoleBufferTest : public GeneralUserObject
{
public:
  static InputParameters validParams();

  ConsoleBufferTest(const InputParameters & params);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
};
