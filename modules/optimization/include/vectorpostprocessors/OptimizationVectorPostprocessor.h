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
class OptimizationVectorPostprocessor;

class OptimizationVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  OptimizationVectorPostprocessor(const InputParameters & parameters);

  virtual void initialSetup() override{};
  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};

  std::vector<std::string> getParameterNames();
  std::vector<Real> getParameterValues();
  void setParameterValues(const VectorPostprocessorValue & current);

private:
  /// The vector variables storing data and name pairs
  std::vector<VectorPostprocessorValue *> _vpp_vectors;
  const std::vector<std::string> & _control_names;
};
