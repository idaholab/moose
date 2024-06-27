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
#include "GeneralVectorPostprocessor.h"
#include "GaussianProcessSurrogate.h"
#include "SurrogateModelInterface.h"

class GaussianProcessData : public GeneralVectorPostprocessor, public SurrogateModelInterface
{
public:
  static InputParameters validParams();

  GaussianProcessData(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}

protected:
  /// Reference to GaussianProcess
  const GaussianProcessSurrogate & _gp_surrogate;

  /// Vector of hyperparamater values
  std::vector<VectorPostprocessorValue *> _hp_vector;
};
