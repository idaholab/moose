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

registerMooseObjectDeprecated("OptimizationApp", OptimizationReporter, "12/31/2024 24:00");

InputParameters
OptimizationReporter::validParams()
{
  InputParameters params = OptimizationDataTempl<OptimizationReporterBase>::validParams();
  params.addClassDescription("Computes objective function, gradient and contains reporters for "
                             "communicating between optimizeSolve and subapps");
  params.addRequiredParam<std::vector<dof_id_type>>(
      "num_values",
      "Number of parameter values associated with each parameter group in 'parameter_names'.");
  params.addParam<std::vector<std::vector<Real>>>(
      "initial_condition",
      "Initial conditions for each parameter. A vector is given for each parameter group.  A "
      "single value can be given for each group and all parameters in that group will be set to "
      "that value.  The default value is 0.");
  params.addParam<std::vector<std::vector<Real>>>(
      "lower_bounds",
      "Lower bound for each parameter.  A vector is given for each parameter group.  A single "
      "value can be given for each group and all parameters in that group will be set to that "
      "value");
  params.addParam<std::vector<std::vector<Real>>>(
      "upper_bounds",
      "Upper bound for each parameter.  A vector is given for each parameter group.  A single "
      "value can be given for each group and all parameters in that group will be set to that "
      "value");
  return params;
}

OptimizationReporter::OptimizationReporter(const InputParameters & parameters)
  : OptimizationDataTempl<OptimizationReporterBase>(parameters)
{
  mooseDeprecated(
      "The 'OptimizationReporter' is deprecated. Please use 'GeneralOptimization' instead. "
      "You can achieve the same functionality by using an 'OptimizationData' object in the "
      "forward application to calculate the objective value, similar to the method used here.");
}
void
OptimizationReporter::setICsandBounds()
{
  _nvalues = getParam<std::vector<dof_id_type>>("num_values");
  _ndof = std::accumulate(_nvalues.begin(), _nvalues.end(), 0);

  // size checks
  if (_parameter_names.size() != _nvalues.size())
    paramError(
        "num_parameters",
        "There should be a number in \'num_parameters\' for each name in \'parameter_names\'.");

  for (const auto & param_id : make_range(_nparams))
  {
    _gradients[param_id]->resize(_nvalues[param_id]);

    std::vector<Real> ic(parseInputData("initial_condition", 0, param_id));
    std::vector<Real> lb(
        parseInputData("lower_bounds", std::numeric_limits<Real>::lowest(), param_id));
    std::vector<Real> ub(
        parseInputData("upper_bounds", std::numeric_limits<Real>::max(), param_id));

    _lower_bounds.insert(_lower_bounds.end(), lb.begin(), lb.end());
    _upper_bounds.insert(_upper_bounds.end(), ub.begin(), ub.end());

    _parameters[param_id]->assign(ic.begin(), ic.end());
  }
}

Real
OptimizationReporter::computeObjective()
{
  // This will only be executed if measurement_values are available on the main app
  for (const auto i : index_range(_measurement_values))
    _misfit_values[i] = _simulation_values[i] - _measurement_values[i];

  Real val = 0.0;
  for (auto & misfit : _misfit_values)
    val += misfit * misfit;

  if (_tikhonov_coeff > 0.0)
  {
    Real param_norm_sqr = 0;
    for (const auto & data : _parameters)
      for (const auto & val : *data)
        param_norm_sqr += val * val;

    val += _tikhonov_coeff * param_norm_sqr;
  }

  return val * 0.5;
}

void
OptimizationReporter::setMisfitToSimulatedValues()
{
  _misfit_values = _simulation_values;
}
