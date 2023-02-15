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

namespace // unnamed namespace
{
void
copyReporterIntoPetscVector(const std::vector<std::vector<Real> *> reporterVectors,
                            libMesh::PetscVector<Number> & x)
{
  dof_id_type n = 0;
  for (const auto & data : reporterVectors)
    for (const auto & val : *data)
      x.set(n++, val);

  x.close();
}

void
copyPetscVectorIntoReporter(const libMesh::PetscVector<Number> & x,
                            std::vector<std::vector<Real> *> reporterVectors)
{
  dof_id_type n = 0;
  for (auto & data : reporterVectors)
    for (auto & val : *data)
      val = x(n++);
}
}

InputParameters
OptimizationReporter::validParams()
{
  InputParameters params = OptimizationReporterBase::validParams();
  params.addClassDescription("Computes objective function, gradient and contains reporters for "
                             "communicating between optimizeSolve and subapps");
  params.addRequiredParam<std::vector<ReporterValueName>>(
      "parameter_names", "List of parameter names, one for each group of parameters.");
  params.addRequiredParam<std::vector<dof_id_type>>(
      "num_values",
      "Number of parameter values associated with each parameter group in 'parameter_names'.");
  params.addParam<std::vector<Real>>("initial_condition",
                                     "Initial condition for each parameter values, default is 0.");
  params.addParam<std::vector<Real>>(
      "lower_bounds", std::vector<Real>(), "Lower bounds for each parameter value.");
  params.addParam<std::vector<Real>>(
      "upper_bounds", std::vector<Real>(), "Upper bounds for each parameter value.");

  return params;
}

OptimizationReporter::OptimizationReporter(const InputParameters & parameters)
  : OptimizationReporterBase(parameters),
    _parameter_names(getParam<std::vector<ReporterValueName>>("parameter_names")),
    _nparams(_parameter_names.size()),
    _parameters(_nparams),
    _gradients(_nparams),
    _lower_bounds(getParam<std::vector<Real>>("lower_bounds")),
    _upper_bounds(getParam<std::vector<Real>>("upper_bounds")),
    _nvalues(getParam<std::vector<dof_id_type>>("num_values")),
    _ndof(std::accumulate(_nvalues.begin(), _nvalues.end(), 0))
{
  if (_parameter_names.size() != _nvalues.size())
    paramError("num_parameters",
               "There should be a number in 'num_parameters' for each name in 'parameter_names'.");
  std::vector<Real> initial_condition = isParamValid("initial_condition")
                                            ? getParam<std::vector<Real>>("initial_condition")
                                            : std::vector<Real>(_ndof, 0.0);
  if (initial_condition.size() != _ndof)
    paramError("initial_condition",
               "Initial condition must be same length as the total number of parameter values.");
  if (_upper_bounds.size() > 0 && _upper_bounds.size() != _ndof)
    paramError("upper_bounds", "Upper bound data is not equal to the total number of parameters.");
  else if (_lower_bounds.size() > 0 && _lower_bounds.size() != _ndof)
    paramError("lower_bounds", "Lower bound data is not equal to the total number of parameters.");
  else if (_lower_bounds.size() != _upper_bounds.size())
    paramError((_lower_bounds.size() == 0 ? "upper_bounds" : "lower_bounds"),
               "Both upper and lower bounds must be specified if bounds are used");

  unsigned int v = 0;
  for (const auto & i : make_range(_nparams))
  {
    _parameters[i] =
        &declareValueByName<std::vector<Real>>(_parameter_names[i], REPORTER_MODE_REPLICATED);

    _parameters[i]->assign(initial_condition.begin() + v,
                           initial_condition.begin() + v + _nvalues[i]);
    _gradients[i] = &declareValueByName<std::vector<Real>>("grad_" + _parameter_names[i],
                                                           REPORTER_MODE_REPLICATED);
    _gradients[i]->resize(_nvalues[i]);
    v += _nvalues[i];
  }
}

void
OptimizationReporter::setInitialCondition(libMesh::PetscVector<Number> & x)
{
  x.init(_ndof);
  copyReporterIntoPetscVector(_parameters, x);
}

void
OptimizationReporter::updateParameters(const libMesh::PetscVector<Number> & x)
{
  copyPetscVectorIntoReporter(x, _parameters);
}

void
OptimizationReporter::computeGradient(libMesh::PetscVector<Number> & gradient) const
{
  for (const auto & p : make_range(_nparams))
    if (_gradients[p]->size() != _nvalues[p])
      mooseError("The gradient for parameter ",
                 _parameter_names[p],
                 " has changed, expected ",
                 _nvalues[p],
                 " versus ",
                 _gradients[p]->size(),
                 ".");

  copyReporterIntoPetscVector(_gradients, gradient);
}
