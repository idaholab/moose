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

InputParameters
OptimizationReporterBase::validParams()
{
  InputParameters params = OptimizationData::validParams();
  params.registerBase("OptimizationReporterBase");
  params.addRequiredParam<std::vector<ReporterValueName>>(
      "parameter_names", "List of parameter names, one for each group of parameters.");
  params.addParam<std::vector<Real>>(
      "lower_bounds", std::vector<Real>(), "Constant lower bound for each group of parameters.");
  params.addParam<std::vector<Real>>(
      "upper_bounds", std::vector<Real>(), "Constant upper bound for each group of parameters.");
  params.registerBase("OptimizationReporterBase");
  return params;
}

OptimizationReporterBase::OptimizationReporterBase(const InputParameters & parameters)
  : OptimizationData(parameters),
    _parameter_names(getParam<std::vector<ReporterValueName>>("parameter_names")),
    _nparams(_parameter_names.size()),
    _parameters(_nparams),
    _gradients(_nparams),
    _lower_bounds(getParam<std::vector<Real>>("lower_bounds")),
    _upper_bounds(getParam<std::vector<Real>>("upper_bounds"))
{
  if (!_lower_bounds.empty() && _lower_bounds.size() != _nparams)
    paramError("lower_bounds", "There must be a lower bound associated with each parameter.");
  else if (!_upper_bounds.empty() && _upper_bounds.size() != _nparams)
    paramError("upper_bounds", "There must be an upper bound associated with each parameter.");
  else if (_lower_bounds.size() != _upper_bounds.size())
    paramError((_lower_bounds.size() == 0 ? "upper_bounds" : "lower_bounds"),
               "Both upper and lower bounds must be specified if bounds are used.");

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
  for (const auto & p : make_range(_nparams))
    if (_gradients[p]->size() != _nvalues[p])
      mooseError("The gradient for parameter ",
                 _parameter_names[p],
                 " has changed, expected ",
                 _nvalues[p],
                 " versus ",
                 _gradients[p]->size(),
                 ".");
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
  return _lower_bounds.empty() ? std::numeric_limits<Real>::lowest()
                               : _lower_bounds[getParameterIndex(i)];
}

Real
OptimizationReporterBase::getUpperBound(dof_id_type i) const
{
  return _upper_bounds.empty() ? std::numeric_limits<Real>::max()
                               : _upper_bounds[getParameterIndex(i)];
}

unsigned int
OptimizationReporterBase::getParameterIndex(dof_id_type i) const
{
  dof_id_type dof = 0;
  for (unsigned int p = 0; p < _nparams; ++p)
  {
    dof += _nvalues[p];
    if (i < dof)
      return p;
  }
  mooseError("DoF index ", i, " is outside of expected paramter vector of size ", _ndof, ".");
  return 0;
}
