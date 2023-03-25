//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationReporter.h"

#include "libmesh/int_range.h"

registerMooseObject("OptimizationApp", OptimizationReporter);

InputParameters
OptimizationReporter::validParams()
{
  InputParameters params = OptimizationReporterBase::validParams();
  params.addClassDescription("Computes objective function, gradient and contains reporters for "
                             "communicating between optimizeSolve and subapps");
  params.addRequiredParam<std::vector<dof_id_type>>(
      "num_values",
      "Number of parameter values associated with each parameter group in 'parameter_names'.");
  params.addParam<std::vector<std::vector<Real>>>(
      "initial_condition", "Initial condition for each parameter value, default is 0.");
  return params;
}

OptimizationReporter::OptimizationReporter(const InputParameters & parameters)
  : OptimizationReporterBase(parameters)
{
  _nvalues = getParam<std::vector<dof_id_type>>("num_values");
  _ndof = std::accumulate(_nvalues.begin(), _nvalues.end(), 0);

  // size checks
  if (_parameter_names.size() != _nvalues.size())
    paramError(
        "num_parameters",
        "There should be a number in \'num_parameters\' for each name in \'parameter_names\'.");

  // fill lower, upper, initial conditions and setup reporters
  fillBounds();
  fillInitialConditions();
  initializeOptimizationReporters();
}
