//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

/**
 * Test object to create vector of vector needed for testing VectorOfVectorRowSum
 */

class VectorOfVectorTestReporter : public GeneralReporter
{
public:
  static InputParameters validParams();

  VectorOfVectorTestReporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override {}

private:
  /// Reporter vector of vector being created
  std::vector<std::vector<Real>> & _vectors;
};
