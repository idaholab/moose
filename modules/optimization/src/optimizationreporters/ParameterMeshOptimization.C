//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParameterMeshOptimization.h"

#include "AddVariableAction.h"
#include "ParameterMesh.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("OptimizationApp", ParameterMeshOptimization);

InputParameters
ParameterMeshOptimization::validParams()
{
  InputParameters params = OptimizationReporterBase::validParams();
  params.addClassDescription(
      "Computes objective function, gradient and contains reporters for communicating between "
      "optimizeSolve and subapps using mesh-based parameter definition.");

  params.addRequiredParam<std::vector<ReporterValueName>>(
      "parameter_names", "List of parameter names, one for each group of parameters.");
  params.addRequiredParam<std::vector<FileName>>(
      "parameter_meshes", "Exodus file containing meshes describing parameters.");

  const auto family = AddVariableAction::getNonlinearVariableFamilies();
  MultiMooseEnum families(family.getRawNames(), "LAGRANGE");
  params.addParam<MultiMooseEnum>(
      "parameter_families",
      families,
      "Specifies the family of FE shape functions for each parameter. If a single value is "
      "specified, then that value is used for all parameters.");
  const auto order = AddVariableAction::getNonlinearVariableOrders();
  MultiMooseEnum orders(order.getRawNames(), "FIRST");
  params.addParam<MultiMooseEnum>(
      "parameter_orders",
      orders,
      "Specifies the order of FE shape functions for each parameter. If a single value is "
      "specified, then that value is used for all parameters.");

  params.addParam<unsigned int>(
      "num_parameter_times", 1, "The number of time points the parameters represent.");

  params.addParam<std::vector<Real>>("initial_condition",
                                     std::vector<Real>(),
                                     "Spatially constant initial condition for each parameter.");
  params.addParam<std::vector<Real>>(
      "lower_bounds", std::vector<Real>(), "Spatially constant lower bound for each parameter.");
  params.addParam<std::vector<Real>>(
      "upper_bounds", std::vector<Real>(), "Spatially constant upper bound for each parameter.");

  return params;
}

ParameterMeshOptimization::ParameterMeshOptimization(const InputParameters & parameters)
  : OptimizationReporterBase(parameters),
    _parameter_names(getParam<std::vector<ReporterValueName>>("parameter_names")),
    _nparams(_parameter_names.size()),
    _nvalues(_nparams),
    _parameters(_nparams),
    _gradients(_nparams),
    _lower_bounds(getParam<std::vector<Real>>("lower_bounds")),
    _upper_bounds(getParam<std::vector<Real>>("upper_bounds"))
{
  const auto & meshes = getParam<std::vector<FileName>>("parameter_meshes");
  const auto & families = getParam<MultiMooseEnum>("parameter_families");
  const auto & orders = getParam<MultiMooseEnum>("parameter_orders");
  const auto & initial_condition = getParam<std::vector<Real>>("initial_condition");
  const auto & ntimes = getParam<unsigned int>("num_parameter_times");

  // Size checks
  if (meshes.size() != _nparams)
    paramError("parameter_meshes", "There must be a mesh associated with each parameter.");
  if (families.size() > 1 && families.size() != _nparams)
    paramError("parameter_families", "There must be a family associated with each parameter.");
  if (orders.size() > 1 && orders.size() != _nparams)
    paramError("parameter_orders", "There must be an order associated with each parameter.");
  if (!initial_condition.empty() && initial_condition.size() != _nparams)
    paramError("initial_condition",
               "There must be an initial condition associated with each parameter.");
  if (!_lower_bounds.empty() && _lower_bounds.size() != _nparams)
    paramError("lower_bounds", "There must be a lower bound associated with each parameter.");
  if (!_upper_bounds.empty() && _upper_bounds.size() != _nparams)
    paramError("upper_bounds", "There must be an upper bound associated with each parameter.");

  _ndof = 0;
  for (const auto & i : make_range(_nparams))
  {
    const std::string family =
        families.size() > 1 ? std::string(families[i]) : std::string(families[0]);
    const std::string order = orders.size() > 1 ? std::string(orders[i]) : std::string(orders[0]);
    const FEType fetype(Utility::string_to_enum<Order>(order),
                        Utility::string_to_enum<FEFamily>(family));

    ParameterMesh pmesh(fetype, meshes[i]);
    _nvalues[i] = pmesh.size() * ntimes;
    _ndof += _nvalues[i];

    _parameters[i] =
        &declareValueByName<std::vector<Real>>(_parameter_names[i], REPORTER_MODE_REPLICATED);
    _parameters[i]->assign(_nvalues[i], initial_condition.empty() ? 0.0 : initial_condition[i]);
    _gradients[i] = &declareValueByName<std::vector<Real>>("grad_" + _parameter_names[i],
                                                           REPORTER_MODE_REPLICATED);
    _gradients[i]->resize(_nvalues[i]);
  }
}

void
ParameterMeshOptimization::setInitialCondition(libMesh::PetscVector<Number> & x)
{
  x.init(_ndof);

  dof_id_type n = 0;
  for (const auto & param : _parameters)
    for (const auto & val : *param)
      x.set(n++, val);

  x.close();
}

void
ParameterMeshOptimization::updateParameters(const libMesh::PetscVector<Number> & x)
{
  dof_id_type n = 0;
  for (auto & param : _parameters)
    for (auto & val : *param)
      val = x(n++);
}

void
ParameterMeshOptimization::computeGradient(libMesh::PetscVector<Number> & gradient) const
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

  dof_id_type n = 0;
  for (const auto & grad : _gradients)
    for (const auto & val : *grad)
      gradient.set(n++, val);

  gradient.close();
}

Real
ParameterMeshOptimization::getLowerBound(dof_id_type i) const
{
  return _lower_bounds.empty() ? -std::numeric_limits<Real>::max()
                               : _lower_bounds[getParameterIndex(i)];
}

Real
ParameterMeshOptimization::getUpperBound(dof_id_type i) const
{
  return _upper_bounds.empty() ? std::numeric_limits<Real>::max()
                               : _upper_bounds[getParameterIndex(i)];
}

unsigned int
ParameterMeshOptimization::getParameterIndex(dof_id_type i) const
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
