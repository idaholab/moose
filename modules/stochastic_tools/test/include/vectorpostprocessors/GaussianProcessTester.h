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
#include "EvaluateSurrogate.h"
#include "GaussianProcess.h"

/**
 * A tool for output Gaussian Process Surrogate data.
 */
class GaussianProcessTester : public EvaluateSurrogate
{
public:
  static InputParameters validParams();

  GaussianProcessTester(const InputParameters & parameters);
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// Pointers to surrogate model
  std::vector<const GaussianProcess *> _GP_model;
  /// Vectors containing standard deviation of the results of sampling model
  std::vector<VectorPostprocessorValue *> _std_vector;
};
