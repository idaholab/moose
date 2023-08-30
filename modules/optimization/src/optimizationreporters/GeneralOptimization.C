//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTypes.h"
#include "GeneralOptimization.h"

registerMooseObject("OptimizationApp", GeneralOptimization);

InputParameters
GeneralOptimization::validParams()
{
  InputParameters params = OptimizationReporterBase::validParams();

  params.addRequiredParam<ReporterValueName>("objective_name", "Objective reporter name.");
  params.addRequiredParam<ReporterValueName>("number_dofs_reporter", " reporter name.");

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

  params.addClassDescription("");
  return params;
}

GeneralOptimization::GeneralOptimization(const InputParameters & parameters)
  : OptimizationReporterBase(parameters),
    _objective_val(declareValueByName<Real>(getParam<ReporterValueName>("objective_name"),
                                            REPORTER_MODE_REPLICATED)),
    _number_dofs(declareValueByName<int>(getParam<ReporterValueName>("number_dofs_reporter"),
                                         REPORTER_MODE_REPLICATED))
{
  setICsandBounds();
}

Real
GeneralOptimization::computeObjective()
{
  return _objective_val;
}

void
GeneralOptimization::setICsandBounds()
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
