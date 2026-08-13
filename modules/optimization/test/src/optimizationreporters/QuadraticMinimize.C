//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadraticMinimize.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/int_range.h"

#include <numeric>
#include <limits>

registerMooseObject("OptimizationTestApp", QuadraticMinimize);

InputParameters
QuadraticMinimize::validParams()
{
  InputParameters params = OptimizationReporterBase::validParams();
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
  params.addRequiredParam<Real>("objective", "Desired value of objective function.");
  params.addRequiredParam<std::vector<Real>>("solution", "Desired solution to optimization.");
  return params;
}

QuadraticMinimize::QuadraticMinimize(const InputParameters & parameters)
  : OptimizationReporterBase(parameters),
    _result(getParam<Real>("objective")),
    _solution(getParam<std::vector<Real>>("solution"))
{
  setICsandBounds();
  if (_solution.size() != _ndof)
    paramError("solution", "Size not equal to number of degrees of freedom (", _ndof, ").");
}

void
QuadraticMinimize::setICsandBounds()
{
  _nvalues = getParam<std::vector<dof_id_type>>("num_values");
  _ndof = std::accumulate(_nvalues.begin(), _nvalues.end(), 0);

  // size checks
  if (_parameter_names.size() != _nvalues.size())
    paramError(
        "num_values",
        "There should be a number in \'num_values\' for each name in \'parameter_names\'.");

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
QuadraticMinimize::computeObjective()
{
  Real obj = _result;
  unsigned int i = 0;
  for (const auto & param : _parameters)
    for (const auto & val : *param)
    {
      Real tmp = val - _solution[i++];
      obj += tmp * tmp;
    }

  return obj;
}

void
QuadraticMinimize::computeGradient(libMesh::PetscVector<Number> & gradient) const
{
  unsigned int i = 0;
  for (const auto & param : _parameters)
    for (const auto & val : *param)
    {
      gradient.set(i, 2.0 * (val - _solution[i]));
      i++;
    }
  gradient.close();
}
