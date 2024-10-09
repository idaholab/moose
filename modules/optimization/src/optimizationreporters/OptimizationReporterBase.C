//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTypes.h"
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
  params.addParam<std::vector<ReporterValueName>>(
      "equality_names", std::vector<ReporterValueName>(), "List of equality names.");
  params.addParam<std::vector<ReporterValueName>>(
      "inequality_names", std::vector<ReporterValueName>(), "List of inequality names.");
  params.registerBase("OptimizationReporterBase");
  return params;
}

OptimizationReporterBase::OptimizationReporterBase(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _parameter_names(getParam<std::vector<ReporterValueName>>("parameter_names")),
    _nparams(_parameter_names.size()),
    _parameters(_nparams),
    _gradients(_nparams),
    _tikhonov_coeff(getParam<Real>("tikhonov_coeff")),
    _equality_names(&getParam<std::vector<ReporterValueName>>("equality_names")),
    _n_eq_cons(_equality_names->size()),
    _eq_constraints(_n_eq_cons),
    _eq_gradients(_n_eq_cons),
    _inequality_names(&getParam<std::vector<ReporterValueName>>("inequality_names")),
    _n_ineq_cons(_inequality_names->size()),
    _ineq_constraints(_n_ineq_cons),
    _ineq_gradients(_n_ineq_cons)
{
  for (const auto & i : make_range(_nparams))
  {
    _parameters[i] =
        &declareValueByName<std::vector<Real>>(_parameter_names[i], REPORTER_MODE_REPLICATED);
    _gradients[i] = &declareValueByName<std::vector<Real>>("grad_" + _parameter_names[i],
                                                           REPORTER_MODE_REPLICATED);
  }
  for (const auto & i : make_range(_n_eq_cons))
  {
    _eq_constraints[i] =
        &declareValueByName<std::vector<Real>>(_equality_names->at(i), REPORTER_MODE_REPLICATED);
    _eq_gradients[i] = &declareValueByName<std::vector<Real>>("grad_" + _equality_names->at(i),
                                                              REPORTER_MODE_REPLICATED);
  }
  for (const auto & i : make_range(_n_ineq_cons))
  {
    _ineq_constraints[i] =
        &declareValueByName<std::vector<Real>>(_inequality_names->at(i), REPORTER_MODE_REPLICATED);
    _ineq_gradients[i] = &declareValueByName<std::vector<Real>>("grad_" + _inequality_names->at(i),
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
  setICsandBounds();
  x.init(getNumParams());
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
OptimizationReporterBase::parseInputData(std::string type,
                                         Real default_value,
                                         unsigned int param_id) const
{
  // fill with default values
  std::vector<Real> parsed_data_id(_nvalues[param_id], default_value);
  if (isParamValid(type))
  {
    std::vector<std::vector<Real>> parsed_data(getParam<std::vector<std::vector<Real>>>(type));
    parsed_data_id.assign(parsed_data[param_id].begin(), parsed_data[param_id].end());
    if (parsed_data.size() != _nvalues.size())
      paramError(type,
                 "There must be a vector of ",
                 type,
                 " per parameter group.  The ",
                 type,
                 " input format is std::vector<std::vector<Real>> so each vector should be "
                 "seperated by \";\" even if it is a single value per group for a constant ",
                 type,
                 ".");
    // The case when the initial condition is constant for each parameter group
    if (parsed_data[param_id].size() == 1)
      parsed_data_id.assign(_nvalues[param_id], parsed_data[param_id][0]);
    else if (parsed_data[param_id].size() != _nvalues[param_id])
      paramError(type,
                 "When ",
                 type,
                 " are given in input file, there must either be a single value per parameter "
                 "group or a value for every parameter in the group.");
  }

  return parsed_data_id;
}

void
OptimizationReporterBase::computeEqualityConstraints(
    libMesh::PetscVector<Number> & eqs_constraints) const
{
  OptUtils::copyReporterIntoPetscVector(_eq_constraints, eqs_constraints);
}

void
OptimizationReporterBase::computeInequalityConstraints(
    libMesh::PetscVector<Number> & ineqs_constraints) const
{
  OptUtils::copyReporterIntoPetscVector(_ineq_constraints, ineqs_constraints);
}

void
OptimizationReporterBase::computeEqualityGradient(libMesh::PetscMatrix<Number> & jacobian) const
{
  for (const auto & p : make_range(_n_eq_cons))
    if (_eq_gradients[p]->size() != _ndof)
      mooseError("The equality jacobian for parameter ",
                 _parameter_names[p],
                 " has changed, expected ",
                 _ndof,
                 " versus ",
                 _eq_gradients[p]->size(),
                 ".");
  OptUtils::copyReporterIntoPetscMatrix(_eq_gradients, jacobian);
}

void
OptimizationReporterBase::computeInequalityGradient(libMesh::PetscMatrix<Number> & jacobian) const
{
  for (const auto & p : make_range(_n_ineq_cons))
    if (_ineq_gradients[p]->size() != _ndof)
      mooseError("The inequality jacobian for parameter ",
                 _parameter_names[p],
                 " has changed, expected ",
                 _ndof,
                 " versus ",
                 _ineq_gradients[p]->size(),
                 ".");
  OptUtils::copyReporterIntoPetscMatrix(_ineq_gradients, jacobian);
}
