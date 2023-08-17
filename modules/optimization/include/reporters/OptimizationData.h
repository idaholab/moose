//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "OptimizationDataHelper.h"
#include "GeneralReporter.h"
/**
 * Reporter to hold measurement and simulation data for optimization problems
 */
class OptimizationData : public GeneralReporter
{
public:
  static InputParameters validParams();

  OptimizationData(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
private:
  OptimizationDataHelper _opt_data;
};
