//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
#include "MooseTypes.h"
#include "GeneralOptimization.h"
#include "libmesh/id_types.h"
#include <numeric>

registerMooseObject("OptimizationApp", GeneralOptimization);

InputParameters
GeneralOptimization::validParams()
{
  InputParameters params = OptimizationReporterBase::validParams();

  params.addRequiredParam<ReporterValueName>(
      "objective_name", "Preferred name of reporter value defining the objective.");
  params.addParam<std::vector<dof_id_type>>(
      "num_values",
      "Number of parameter values associated with each parameter group in 'parameter_names'.");
  params.addParam<ReporterValueName>("num_values_name",
                                     "Reporter that holds the number of parameter values "
                                     "associated with each parameter group in 'parameter_names'.");
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

  params.addClassDescription("Reporter that provides TAO with the objective, gradient, and "
                             "constraint data, which are supplied by the reporters and "
                             "postprocessors from the forward and adjoint subapps.");
  return params;
}

GeneralOptimization::GeneralOptimization(const InputParameters & parameters)
  : OptimizationReporterBase(parameters),
    _objective_val(declareValueByName<Real>(getParam<ReporterValueName>("objective_name"),
                                            REPORTER_MODE_REPLICATED)),
    _num_values_reporter(
        isParamValid("num_values_name")
            ? &declareValueByName<std::vector<dof_id_type>>(
                  getParam<ReporterValueName>("num_values_name"), REPORTER_MODE_REPLICATED)
            : nullptr)
{
}

Real
GeneralOptimization::computeObjective()
{
  Real val = 0;
  if (_tikhonov_coeff > 0.0)
  {
    Real param_norm_sqr = 0;
    for (const auto & data : _parameters)
      for (const auto & param_val : *data)
        param_norm_sqr += param_val * param_val;
    // We multiply by 0.5 to maintain  backwards compatibility.
    val += 0.5 * _tikhonov_coeff * param_norm_sqr;
  }
  return _objective_val + val;
}

dof_id_type
GeneralOptimization::getNumParams() const
{
  if (_ndof == 0)
    mooseError(
        "The number of parameters you have is zero and this shouldn't happen. Make sure you are "
        "running your forward problem on \'INITIAL\' if you are using a reporter transfer to "
        "supply "
        "that information.");

  return _ndof;
}

void
GeneralOptimization::setICsandBounds()
{
  // Check that one and only one set of parameters are given.
  // Set here because some derived reporters use a different method of
  // determining numbers of dofs
  if (!(isParamValid("num_values_name") ^ isParamValid("num_values")))
    paramError("Need to supply one and only one of num_values_name or num_values.");

  if (_num_values_reporter)
    _nvalues = *_num_values_reporter;
  else
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
