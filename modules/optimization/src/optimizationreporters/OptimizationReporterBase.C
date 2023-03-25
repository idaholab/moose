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
  params.addParam<std::vector<std::vector<Real>>>(
      "lower_bounds", "Constant lower bound for each group of parameters.");
  params.addParam<std::vector<std::vector<Real>>>(
      "upper_bounds", "Constant upper bound for each group of parameters.");
  params.registerBase("OptimizationReporterBase");
  return params;
}

OptimizationReporterBase::OptimizationReporterBase(const InputParameters & parameters)
  : OptimizationData(parameters),
    _parameter_names(getParam<std::vector<ReporterValueName>>("parameter_names")),
    _nparams(_parameter_names.size()),
    _parameters(_nparams),
    _gradients(_nparams)
{
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
OptimizationReporterBase::getLowerBound(dof_id_type index) const
{
  std::pair<std::size_t, std::size_t> ij = getParameterIndex(index);
  return _lower_bounds.empty() ? std::numeric_limits<Real>::lowest()
                               : _lower_bounds[ij.first][ij.second];
}

Real
OptimizationReporterBase::getUpperBound(dof_id_type index) const
{
  std::pair<std::size_t, std::size_t> ij = getParameterIndex(index);
  return _upper_bounds.empty() ? std::numeric_limits<Real>::max()
                               : _upper_bounds[ij.first][ij.second];
}

std::pair<std::size_t, std::size_t>
OptimizationReporterBase::getParameterIndex(dof_id_type index) const
{
  std::pair<std::size_t, std::size_t> ij;
  dof_id_type params_per_group = 0;
  for (std::size_t i = 0; i < _nparams; ++i)
  {
    params_per_group += _nvalues[i];
    if (index < params_per_group)
    {
      ij.first = i;
      ij.second = index - (params_per_group - _nvalues[i]);
      return ij;
    }
  }
  mooseError("DoF index ", index, " is outside of expected paramter vector of size ", _ndof, ".");
  return ij;
}

void
OptimizationReporterBase::fillBounds()
{
  _lower_bounds = fillVectorOfVectors("lower_bounds");
  _upper_bounds = fillVectorOfVectors("upper_bounds");

  Real defaultValue = std::numeric_limits<Real>::lowest();
  fillWithDefaults(defaultValue, _lower_bounds);
  defaultValue = std::numeric_limits<Real>::max();
  fillWithDefaults(defaultValue, _upper_bounds);

  if (!_lower_bounds.empty() && _lower_bounds.size() != _nparams)
    paramError("lower_bounds", "There must be a lower bound associated with each parameter.");
  else if (!_upper_bounds.empty() && _upper_bounds.size() != _nparams)
    paramError("upper_bounds", "There must be an upper bound associated with each parameter.");
}

void
OptimizationReporterBase::fillInitialConditions()
{
  _initial_conditions = fillVectorOfVectors("initial_condition");
  Real defaultValue = 0.0;
  fillWithDefaults(defaultValue, _initial_conditions);
}

void
OptimizationReporterBase::initializeOptimizationReporters()
{
  for (const auto & i : make_range(_nparams))
  {
    _parameters[i] =
        &declareValueByName<std::vector<Real>>(_parameter_names[i], REPORTER_MODE_REPLICATED);
    _parameters[i]->assign(_initial_conditions[i].begin(), _initial_conditions[i].end());
    _gradients[i] = &declareValueByName<std::vector<Real>>("grad_" + _parameter_names[i],
                                                           REPORTER_MODE_REPLICATED);
    _gradients[i]->resize(_nvalues[i]);
  }
}

void
OptimizationReporterBase::fillWithDefaults(Real defaultValue,
                                           std::vector<std::vector<Real>> & data) const
{
  if (data.empty())
  {
    // fill with zeros because initial conditions are not given in input file
    for (auto & paramsPerGroup : _nvalues)
    {
      std::vector<Real> paramGroupDefaults(paramsPerGroup, defaultValue);
      data.push_back(paramGroupDefaults);
    }
  }
}

std::vector<std::vector<Real>>
OptimizationReporterBase::fillVectorOfVectors(std::string type) const
{
  std::vector<std::vector<Real>> parsedData;
  if (isParamValid(type))
  {
    parsedData = getParam<std::vector<std::vector<Real>>>(type);
    if (parsedData.size() != _nvalues.size())
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
    for (std::size_t i = 0; i < parsedData.size(); ++i)
    {
      // The case when the initial condition is constant for each parameter group
      if ((parsedData[i].size() == 1) && (parsedData[i].size() < _nvalues[i]))
      {
        std::vector<Real> params_group_constant(_nvalues[i], parsedData[i][0]);
        parsedData[i] = params_group_constant;
      }
      else if ((parsedData[i].size() != 1) && (parsedData[i].size() != _nvalues[i]))
      {
        paramError(type,
                   "When ",
                   type,
                   " are given in input file, there must either be a single value per parameter "
                   "group or a value for every parameter.");
      }
    }
  }

  return parsedData;
}
