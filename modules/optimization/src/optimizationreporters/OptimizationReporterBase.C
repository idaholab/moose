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
  InputParameters params = OptimizationData::validParams();
  params.registerBase("OptimizationReporterBase");
  params.addRequiredParam<std::vector<ReporterValueName>>(
      "parameter_names", "List of parameter names, one for each group of parameters.");
  params.addRangeCheckedParam<Real>(
      "tikhonov_coeff", 0.0, "tikhonov_coeff >= 0", "Coefficient for Tikhonov Regularization.");
  params.suppressParameter<std::vector<VariableName>>("variable");
  params.suppressParameter<std::vector<std::string>>("variable_weight_names");
  params.registerBase("OptimizationReporterBase");
  return params;
}

OptimizationReporterBase::OptimizationReporterBase(const InputParameters & parameters)
  : OptimizationData(parameters),
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

Real
OptimizationReporterBase::computeObjective()
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
OptimizationReporterBase::setMisfitToSimulatedValues()
{
  _misfit_values = _simulation_values;
}

// function only used for test objects
void
OptimizationReporterBase::setSimulationValuesForTesting(std::vector<Real> & data)
{
  _simulation_values.clear();
  _simulation_values = data;
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
