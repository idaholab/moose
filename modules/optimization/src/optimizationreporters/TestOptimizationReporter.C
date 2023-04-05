//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestOptimizationReporter.h"

registerMooseObject("OptimizationApp", TestOptimizationReporter);

InputParameters
TestOptimizationReporter::validParams()
{
  InputParameters params = OptimizationReporter::validParams();
  params.addClassDescription("");

  return params;
}

TestOptimizationReporter::TestOptimizationReporter(const InputParameters & parameters)
  : OptimizationReporter(parameters)
{
  // set the initial conditions manually
  (*_parameters[0])[0] = 1;
  (*_parameters[0])[1] = 0;
}

Real
TestOptimizationReporter::computeObjective()
{
  std::vector<Real> x(2);
  x[0] = (*_parameters[0])[0];
  x[1] = (*_parameters[0])[1];
  return 10.0 * std::sin(x[0]) - 0.05 * (x[0] + 2.0) + std::pow(x[0] - 1.0, 2) + 20.0 +
              1.78E-6 * std::pow(x[1], 8) + 1.86E-5 * std::pow(x[1], 7) - 3.75E-4 * std::pow(x[1], 6) -
              3.61E-3 * std::pow(x[1], 5) + 2.55E-2 * std::pow(x[1], 4) + 2.06E-1 * std::pow(x[1], 3) -
              4.85E-1 * std::pow(x[1], 2) - 3.11E0 * x[1] + 1.38E0 + 20.E0;
}
