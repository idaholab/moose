//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "GaussianProcessSurrogateTorched.h"
#include "SurrogateModelInterface.h"

class GaussianProcessDataTorched : public GeneralVectorPostprocessor, public SurrogateModelInterface
{
public:
  static InputParameters validParams();

  GaussianProcessDataTorched(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}

protected:
  /// Reference to GaussianProcess
  const GaussianProcessSurrogateTorched & _gp_surrogate;

  /// Vector of hyperparamater values
  std::vector<VectorPostprocessorValue *> _hp_vector;
};
