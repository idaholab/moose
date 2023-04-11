//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OptimizationReporterBase.h"

/**
 * Computes gradient and contains reporters for communicating between optimizeSolve and subapps
 */
class OptimizationReporter : public OptimizationReporterBase
{
public:
  static InputParameters validParams();
  OptimizationReporter(const InputParameters & parameters);

private:
  std::vector<Real> fillParamsVector(std::string type, Real default_value) const;
};
