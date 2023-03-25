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

  params.addRequiredParam<std::vector<FileName>>(
      "parameter_meshes", "Exodus file containing meshes describing parameters.");

  const auto family = AddVariableAction::getNonlinearVariableFamilies();
  MultiMooseEnum families(family.getRawNames(), "LAGRANGE");
  params.addParam<MultiMooseEnum>(
      "parameter_families",
      families,
      "Specifies the family of FE shape functions for each group of parameters. If a single value "
      "is "
      "specified, then that value is used for all groups of parameters.");
  const auto order = AddVariableAction::getNonlinearVariableOrders();
  MultiMooseEnum orders(order.getRawNames(), "FIRST");
  params.addParam<MultiMooseEnum>(
      "parameter_orders",
      orders,
      "Specifies the order of FE shape functions for each group of parameters. If a single value "
      "is "
      "specified, then that value is used for all groups of parameters.");

  params.addParam<unsigned int>(
      "num_parameter_times", 1, "The number of time points the parameters represent.");

  params.addParam<std::vector<std::vector<Real>>>(
      "initial_condition", "Spatially constant initial condition for each group of parameters.");
  return params;
}

ParameterMeshOptimization::ParameterMeshOptimization(const InputParameters & parameters)
  : OptimizationReporterBase(parameters)
{
  _nvalues.resize(_nparams, 0);

  const auto & meshes = getParam<std::vector<FileName>>("parameter_meshes");
  const auto & families = getParam<MultiMooseEnum>("parameter_families");
  const auto & orders = getParam<MultiMooseEnum>("parameter_orders");
  // const auto & initial_condition = getParam<std::vector<Real>>("initial_condition");
  const auto & ntimes = getParam<unsigned int>("num_parameter_times");

  // Size checks
  if (meshes.size() != _nparams)
    paramError("parameter_meshes",
               "There must be a mesh associated with each group of parameters.");
  if (families.size() > 1 && families.size() != _nparams)
    paramError("parameter_families",
               "There must be a family associated with each group of parameters.");
  if (orders.size() > 1 && orders.size() != _nparams)
    paramError("parameter_orders",
               "There must be an order associated with each group of parameters.");

  _ndof = 0;
  for (const auto & i : make_range(_nparams))
  {
    const std::string family = families.size() > 1 ? families[i] : families[0];
    const std::string order = orders.size() > 1 ? orders[i] : orders[0];
    const FEType fetype(Utility::string_to_enum<Order>(order),
                        Utility::string_to_enum<FEFamily>(family));

    ParameterMesh pmesh(fetype, meshes[i]);
    _nvalues[i] = pmesh.size() * ntimes;
    _ndof += _nvalues[i];
  }

  // fill lower, upper, initial conditions and setup reporters
  fillBounds();
  fillInitialConditions();
  initializeOptimizationReporters();
}
