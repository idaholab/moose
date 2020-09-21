//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class OptimizationResults;

class OptimizationResults : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  OptimizationResults(const InputParameters & parameters);

  virtual void initialSetup() override {};
  virtual void initialize() override {};
  virtual void execute() override {};
  virtual void finalize() override {};

  void setParameterValues(const VectorPostprocessorValue & current);

protected:

  VectorPostprocessorValue & _results;

};
