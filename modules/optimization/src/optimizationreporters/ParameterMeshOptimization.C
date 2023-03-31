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

  params.addParam<std::vector<Real>>("constant_group_initial_condition",
                                     "Constant initial condition for each group of parameters.");
  params.addParam<std::vector<Real>>("constant_group_lower_bounds",
                                     "Constant lower bound for each group of parameters.");
  params.addParam<std::vector<Real>>("constant_group_upper_bounds",
                                     "Constant upper bound for each group of parameters.");
  params.addParam<std::vector<std::string>>(
      "initial_condition_mesh_variable",
      "Name of variable on parameter mesh to use as initial condition.");
  params.addParam<std::vector<std::string>>(
      "lower_bound_mesh_variable", "Name of variable on parameter mesh to use as lower bound.");
  params.addParam<std::vector<std::string>>(
      "upper_bound_mesh_variable", "Name of variable on parameter mesh to use as upper bound.");
  params.addParam<std::vector<unsigned int>>(
      "exodus_timesteps_for_parameter_mesh_variable",
      "Timesteps to read all parameter group bounds and initial conditions from Exodus mesh.  The "
      "options are to give no timestep, a single timestep or \"num_parameter_times\" timesteps.  "
      "No timestep results in the final timestep from the mesh being used.  A single timestep "
      "results in values at that timestep being used for all timesteps.  \"num_parameter_times\" "
      "timesteps results in values from the mesh at those steps being used.  The same timesteps "
      "are used for all parameter groups and all meshes, the capability to define different "
      "timesteps for different meshes is not supported.");

  return params;
}

ParameterMeshOptimization::ParameterMeshOptimization(const InputParameters & parameters)
  : OptimizationReporterBase(parameters)
{

  _nvalues.resize(_nparams, 0);
  // Fill the mesh information
  const auto & meshes = getParam<std::vector<FileName>>("parameter_meshes");
  const auto & families = getParam<MultiMooseEnum>("parameter_families");
  const auto & orders = getParam<MultiMooseEnum>("parameter_orders");
  const auto & ntimes = getParam<unsigned int>("num_parameter_times");

  // Fill input file parameter bounds and IC information
  const std::vector<Real> empty_vec = {};
  std::vector<Real> constant_group_initial_condition(
      isParamValid("constant_group_initial_condition")
          ? getParam<std::vector<Real>>("constant_group_initial_condition")
          : empty_vec);
  std::vector<Real> constant_group_lower_bounds(
      isParamValid("constant_group_lower_bounds")
          ? getParam<std::vector<Real>>("constant_group_lower_bounds")
          : empty_vec);
  std::vector<Real> constant_group_upper_bounds(
      isParamValid("constant_group_upper_bounds")
          ? getParam<std::vector<Real>>("constant_group_upper_bounds")
          : empty_vec);

  // Fill exodus parameter bounds and IC information
  const std::vector<std::string> empty_string_vec(_nparams);
  std::vector<std::string> initial_condition_mesh_variable(
      isParamValid("initial_condition_mesh_variable")
          ? getParam<std::vector<std::string>>("initial_condition_mesh_variable")
          : empty_string_vec);
  std::vector<std::string> lower_bound_mesh_variable(
      isParamValid("lower_bound_mesh_variable")
          ? getParam<std::vector<std::string>>("lower_bound_mesh_variable")
          : empty_string_vec);
  std::vector<std::string> upper_bound_mesh_variable(
      isParamValid("upper_bound_mesh_variable")
          ? getParam<std::vector<std::string>>("upper_bound_mesh_variable")
          : empty_string_vec);

  std::vector<unsigned int> exodus_timestep;
  if (isParamValid("exodus_timesteps_for_parameter_mesh_variable"))
    exodus_timestep =
        getParam<std::vector<unsigned int>>("exodus_timesteps_for_parameter_mesh_variable");
  else
    exodus_timestep = {std::numeric_limits<unsigned int>::max()};

  // now do a bunch of error checking
  // Size checks for data
  if (meshes.size() != _nparams)
    paramError("parameter_meshes",
               "There must be a mesh associated with each group of parameters.");
  if (families.size() > 1 && families.size() != _nparams)
    paramError("parameter_families",
               "There must be a family associated with each group of parameters.");
  if (orders.size() > 1 && orders.size() != _nparams)
    paramError("parameter_orders",
               "There must be an order associated with each group of parameters.");

  // Size checks for input file parameter data
  if (!constant_group_initial_condition.empty() &&
      constant_group_initial_condition.size() != _nparams)
    paramError("constant_group_initial_condition",
               "There must be an initial condition associated with each group of parameters.");
  else if (!constant_group_lower_bounds.empty() && constant_group_lower_bounds.size() != _nparams)
    paramError("constant_group_lower_bounds",
               "There must be a lower bound associated with each parameter.");
  else if (!constant_group_upper_bounds.empty() && constant_group_upper_bounds.size() != _nparams)
    paramError("constant_group_upper_bounds",
               "There must be an upper bound associated with each parameter.");

  // error checking that initial conditions and bounds are only read from a single location
  if (!initial_condition_mesh_variable[0].empty() && !constant_group_initial_condition.empty())
    paramError("constant_group_initial_condition",
               "Initial conditions for all parameter groups can only be defined by "
               "initial_condition_mesh_variable or "
               "constant_group_initial_condition but not both.");
  else if (!lower_bound_mesh_variable[0].empty() && !constant_group_lower_bounds.empty())
    paramError(
        "constant_group_lower_bounds",
        "Lower bounds for all parameter groups can only be defined by lower_bound_mesh_variable or "
        "constant_group_lower_bounds but not both.");
  else if (!upper_bound_mesh_variable[0].empty() && !constant_group_upper_bounds.empty())
    paramError(
        "constant_group_upper_bounds",
        "Upper bounds for all parameter groups can only be defined by upper_bound_mesh_variable or "
        "constant_group_upper_bounds but not both.");

  // Make sure they did not specify too many timesteps
  if (isParamValid("exodus_timesteps_for_parameter_mesh_variable") &&
      ((lower_bound_mesh_variable[0].empty() + upper_bound_mesh_variable[0].empty() +
        initial_condition_mesh_variable[0].empty()) == 3))
    paramError("\"exodus_timesteps_for_parameter_mesh_variable\" should only be specified if "
               "reading values from a mesh.");
  else if (exodus_timestep.size() != ntimes && exodus_timestep.size() != 1)
    paramError("exodus_timesteps_for_parameter_mesh_variable",
               "Number of timesteps to read mesh data specified by "
               "\"exodus_timesteps_for_parameter_mesh_variable\" incorrect. "
               "\"exodus_timesteps_for_parameter_mesh_variable\" can specify a single timestep or "
               "\"num_parameter_times\" timesteps.");

  _ndof = 0;
  for (const auto & i : make_range(_nparams))
  {
    // store off all the variable names that you might want to read from the mesh
    std::vector<std::string> var_names;
    if (!initial_condition_mesh_variable[i].empty())
      var_names.push_back(initial_condition_mesh_variable[i]);
    if (!lower_bound_mesh_variable[i].empty())
      var_names.push_back(lower_bound_mesh_variable[i]);
    if (!upper_bound_mesh_variable[i].empty())
      var_names.push_back(upper_bound_mesh_variable[i]);

    const std::string family = families.size() > 1 ? families[i] : families[0];
    const std::string order = orders.size() > 1 ? orders[i] : orders[0];
    const FEType fetype(Utility::string_to_enum<Order>(order),
                        Utility::string_to_enum<FEFamily>(family));

    ParameterMesh pmesh(fetype, meshes[i], var_names);
    _nvalues[i] = pmesh.size() * ntimes;
    _ndof += _nvalues[i];

    // read and assign initial conditions
    const Real defaultIC = 0.0;
    const Real constant_valueIC(
        constant_group_initial_condition.empty() ? defaultIC : constant_group_initial_condition[i]);
    std::vector<Real> initial_condition = parseData(
        exodus_timestep, pmesh, constant_valueIC, initial_condition_mesh_variable[i], ntimes);
    _parameters[i]->assign(initial_condition.begin(), initial_condition.end());

    // read and assign lower bound
    const Real defaultLB = std::numeric_limits<Real>::lowest();
    const Real constant_valueLB(
        constant_group_lower_bounds.empty() ? defaultLB : constant_group_lower_bounds[i]);
    std::vector<Real> lower_bound =
        parseData(exodus_timestep, pmesh, constant_valueLB, lower_bound_mesh_variable[i], ntimes);
    _lower_bounds.insert(_lower_bounds.end(), lower_bound.begin(), lower_bound.end());

    // read and assign upper bound
    const Real defaultUB = std::numeric_limits<Real>::max();
    const Real constant_valueUB(
        constant_group_upper_bounds.empty() ? defaultUB : constant_group_upper_bounds[i]);
    std::vector<Real> upper_bound =
        parseData(exodus_timestep, pmesh, constant_valueUB, upper_bound_mesh_variable[i], ntimes);
    _upper_bounds.insert(_upper_bounds.end(), upper_bound.begin(), upper_bound.end());

    // resize gradient vector to be filled later
    _gradients[i]->resize(_nvalues[i]);
  }
}
std::vector<Real>
ParameterMeshOptimization::parseData(const std::vector<unsigned int> & exodus_timestep,
                                     const ParameterMesh & pmesh,
                                     Real constantDataFromInput,
                                     const std::string & meshVarName,
                                     unsigned int ntimes) const
{
  unsigned int numberOfControllableParameters = pmesh.size() * ntimes;
  std::vector<Real> parsedData;
  // read from mesh
  if (!meshVarName.empty())
  {
    for (auto const & step : exodus_timestep)
    {
      std::vector<Real> data = pmesh.getParameterValues(meshVarName, step);
      parsedData.insert(parsedData.end(), data.begin(), data.end());
    }
    if (parsedData.size() != numberOfControllableParameters)
      mooseError("Number of parameters assigned by ",
                 meshVarName,
                 " is not equal to the number of parameters on the mesh.  Mesh contains ",
                 numberOfControllableParameters,
                 " parameters and ",
                 meshVarName,
                 " assigned ",
                 parsedData.size(),
                 " parameters.");
  }
  else // read in constant or default values
    parsedData.resize(parsedData.size() + numberOfControllableParameters, constantDataFromInput);

  return parsedData;
}
