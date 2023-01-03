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

class OptimizationFunction;

class OptimizationFunctionTest : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  OptimizationFunctionTest(const InputParameters & params);

  void initialSetup() override {}
  void initialize() override {}
  void execute() override;
  void finalize() override {}

protected:
  const std::vector<Point> & _points;
  const std::vector<Real> & _times;
  std::map<const OptimizationFunction *, std::vector<VectorPostprocessorValue *>> _functions;
};
