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
  : OptimizationReporterBase(parameters)
{
  _nvalues = getParam<std::vector<dof_id_type>>("num_values");
  _ndof = std::accumulate(_nvalues.begin(), _nvalues.end(), 0);

  // size checks
  if (_parameter_names.size() != _nvalues.size())
    paramError(
        "num_parameters",
        "There should be a number in \'num_parameters\' for each name in \'parameter_names\'.");

  std::vector<Real> initial_conditions(fillParamsVector("initial_condition", 0));
  _lower_bounds = fillParamsVector("lower_bounds", std::numeric_limits<Real>::lowest());
  _upper_bounds = fillParamsVector("upper_bounds", std::numeric_limits<Real>::max());

  std::size_t stride = 0;
  for (const auto & i : make_range(_nparams))
  {
    _gradients[i]->resize(_nvalues[i]);
    _parameters[i]->assign(initial_conditions.begin() + stride,
                           initial_conditions.begin() + stride + _nvalues[i]);
    stride += _nvalues[i];
  }
}

std::vector<Real>
OptimizationReporter::fillParamsVector(std::string type, Real default_value) const
{
  std::vector<std::vector<Real>> parsed_data;
  if (isParamValid(type))
  {
    parsed_data = getParam<std::vector<std::vector<Real>>>(type);
    if (parsed_data.size() != _nvalues.size())
    {
      paramError(type,
                 "There must be a vector of ",
                 type,
                 " per parameter group.  The ",
                 type,
                 " input format is std::vector<std::vector<Real>> so each vector should be "
                 "seperated by \";\" even if it is a single value per group for a constant ",
                 type,
                 ".");
    }
    for (std::size_t i = 0; i < parsed_data.size(); ++i)
    {
      // The case when the initial condition is constant for each parameter group
      if (parsed_data[i].size() == 1)
        parsed_data[i].resize(_nvalues[i], parsed_data[i][0]);
      else if (parsed_data[i].size() != _nvalues[i])
        paramError(type,
                   "When ",
                   type,
                   " are given in input file, there must either be a single value per parameter "
                   "group or a value for every parameter in the group.");
    }
  }

  // fill with default values
  if (parsed_data.empty())
    for (const auto & params_per_group : _nvalues)
      parsed_data.emplace_back(params_per_group, default_value);

  // flatten into single vector
  std::vector<Real> flattened_data;
  for (const auto & vec : parsed_data)
    flattened_data.insert(flattened_data.end(), vec.begin(), vec.end());

  return flattened_data;
}
