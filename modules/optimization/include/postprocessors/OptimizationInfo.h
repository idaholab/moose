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
#include "OptimizeSolve.h"

class Optimize;

class OptimizationInfo : public GeneralReporter
{
public:
  static InputParameters validParams();

  OptimizationInfo(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override {}

private:
  Optimize * _optimization_executioner;
  const MultiMooseEnum & _items;

  int & _currentIterate;
  int & _objectiveIterate;
  int & _gradientIterate;
  int & _hessianIterate;
  double & _functionValue;
  double & _gnorm;
  double & _cnorm;
  double & _xdiff;

  // Helper to perform optional declaration based on "_items" from MeshInfo.h
  template <typename T>
  T & declareHelper(const std::string & item_name, const ReporterMode mode);
};

template <typename T>
T &
OptimizationInfo::declareHelper(const std::string & item_name, const ReporterMode mode)
{
  if (!_items.isValid() || _items.contains(item_name))
  {
    return declareValueByName<T>(item_name, mode);
  }

  return declareUnusedValue<T>();
}
