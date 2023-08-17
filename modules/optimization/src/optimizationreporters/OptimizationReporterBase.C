//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationReporterBase.h"
#include "OptUtils.h"
#include "libmesh/petsc_vector.h"

InputParameters
OptimizationReporterBase::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.registerBase("OptimizationReporterBase");
  params.addRequiredParam<std::vector<ReporterValueName>>(
      "parameter_names", "List of parameter names, one for each group of parameters.");
  params.addRangeCheckedParam<Real>(
      "tikhonov_coeff", 0.0, "tikhonov_coeff >= 0", "Coefficient for Tikhonov Regularization.");
  params.registerBase("OptimizationReporterBase");
  return params;
}

OptimizationReporterBase::OptimizationReporterBase(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _parameter_names(getParam<std::vector<ReporterValueName>>("parameter_names")),
    _nparams(_parameter_names.size()),
    _parameters(_nparams),
    _gradients(_nparams),
    _tikhonov_coeff(getParam<Real>("tikhonov_coeff"))
{
  for (const auto & i : make_range(_nparams))
  {
    _parameters[i] =
        &declareValueByName<std::vector<Real>>(_parameter_names[i], REPORTER_MODE_REPLICATED);
    _gradients[i] = &declareValueByName<std::vector<Real>>("grad_" + _parameter_names[i],
                                                           REPORTER_MODE_REPLICATED);
  }
}


void
OptimizationReporterBase::computeGradient(libMesh::PetscVector<Number> & gradient) const
{
  for (const auto & param_group_id : make_range(_nparams))
  {
    if (_gradients[param_group_id]->size() != _nvalues[param_group_id])
      mooseError("The gradient for parameter ",
                 _parameter_names[param_group_id],
                 " has changed, expected ",
                 _nvalues[param_group_id],
                 " versus ",
                 _gradients[param_group_id]->size(),
                 ".");

    if (_tikhonov_coeff > 0.0)
    {
      auto params = _parameters[param_group_id];
      auto grads = _gradients[param_group_id];
      for (const auto & param_id : make_range(_nvalues[param_group_id]))
        (*grads)[param_id] += (*params)[param_id] * _tikhonov_coeff;
    }
  }

  OptUtils::copyReporterIntoPetscVector(_gradients, gradient);
}

void
OptimizationReporterBase::setInitialCondition(libMesh::PetscVector<Number> & x)
{
  x.init(_ndof);
  OptUtils::copyReporterIntoPetscVector(_parameters, x);
}

void
OptimizationReporterBase::updateParameters(const libMesh::PetscVector<Number> & x)
{
  OptUtils::copyPetscVectorIntoReporter(x, _parameters);
}

Real
OptimizationReporterBase::getLowerBound(dof_id_type i) const
{
  return _lower_bounds[i];
}

Real
OptimizationReporterBase::getUpperBound(dof_id_type i) const
{
  return _upper_bounds[i];
}

std::vector<Real>
OptimizationReporterBase::fillParamsVector(std::string type, Real default_value) const
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
