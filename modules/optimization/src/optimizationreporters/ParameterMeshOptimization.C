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

using namespace libMesh;

registerMooseObject("OptimizationApp", ParameterMeshOptimization);

InputParameters
ParameterMeshOptimization::validParams()
{
  InputParameters params = GeneralOptimization::validParams();

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
  : GeneralOptimization(parameters)
{
}

std::vector<Real>
ParameterMeshOptimization::parseExodusData(const std::vector<unsigned int> & exodus_timestep,
                                           const ParameterMesh & pmesh,
                                           const std::string & mesh_var_name,
                                           unsigned int ntimes) const
{
  unsigned int num_cont_params = pmesh.size() * ntimes;
  std::vector<Real> parsed_data;
  // read from mesh

  for (auto const & step : exodus_timestep)
  {
    std::vector<Real> data = pmesh.getParameterValues(mesh_var_name, step);
    parsed_data.insert(parsed_data.end(), data.begin(), data.end());
  }
  if (parsed_data.size() != num_cont_params)
    mooseError("Number of parameters assigned by ",
               mesh_var_name,
               " is not equal to the number of parameters on the mesh.  Mesh contains ",
               num_cont_params,
               " parameters and ",
               mesh_var_name,
               " assigned ",
               parsed_data.size(),
               " parameters.");

  return parsed_data;
}

void
ParameterMeshOptimization::setICsandBounds()
{
  if ((isParamValid("num_values_name") || isParamValid("num_values")))
    paramError("num_values_name or num_values should not be used with ParameterMeshOptimization. "
               "Instead the number of dofs is set by the parameter meshes.");

  _nvalues.resize(_nparams, 0);
  // Fill the mesh information
  const auto & meshes = getParam<std::vector<FileName>>("parameter_meshes");
  const auto & families = getParam<MultiMooseEnum>("parameter_families");
  const auto & orders = getParam<MultiMooseEnum>("parameter_orders");
  const auto & ntimes = getParam<unsigned int>("num_parameter_times");

  // Fill exodus parameter bounds and IC information
  std::vector<std::string> initial_condition_mesh_variable;
  std::vector<std::string> lower_bound_mesh_variable;
  std::vector<std::string> upper_bound_mesh_variable;
  if (isParamValid("initial_condition_mesh_variable"))
    initial_condition_mesh_variable =
        getParam<std::vector<std::string>>("initial_condition_mesh_variable");
  if (isParamValid("lower_bound_mesh_variable"))
    lower_bound_mesh_variable = getParam<std::vector<std::string>>("lower_bound_mesh_variable");
  if (isParamValid("upper_bound_mesh_variable"))
    upper_bound_mesh_variable = getParam<std::vector<std::string>>("upper_bound_mesh_variable");

  std::vector<unsigned int> exodus_timestep;
  if (isParamValid("exodus_timesteps_for_parameter_mesh_variable"))
    exodus_timestep =
        getParam<std::vector<unsigned int>>("exodus_timesteps_for_parameter_mesh_variable");
  else // get last timestep in file
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

  // error checking that initial conditions and bounds are only read from a single location
  if (isParamValid("initial_condition_mesh_variable") && isParamValid("initial_condition"))
    paramError("initial_condition_mesh_variable",
               "Initial conditions for all parameter groups can only be defined by "
               "initial_condition_mesh_variable or "
               "initial_condition but not both.");
  else if (isParamValid("lower_bound_mesh_variable") && isParamValid("lower_bounds"))
    paramError(
        "lower_bound_mesh_variable",
        "Lower bounds for all parameter groups can only be defined by lower_bound_mesh_variable or "
        "lower_bounds but not both.");
  else if (isParamValid("upper_bound_mesh_variable") && isParamValid("upper_bounds"))
    paramError(
        "upper_bound_mesh_variable",
        "Upper bounds for all parameter groups can only be defined by upper_bound_mesh_variable or "
        "upper_bounds but not both.");

  // Make sure they did not specify too many timesteps
  if (isParamValid("exodus_timesteps_for_parameter_mesh_variable") &&
      (!isParamValid("lower_bound_mesh_variable") + !isParamValid("upper_bound_mesh_variable") +
           !isParamValid("initial_condition_mesh_variable") ==
       3))
    paramError("\"exodus_timesteps_for_parameter_mesh_variable\" should only be specified if "
               "reading values from a mesh.");
  else if (exodus_timestep.size() != ntimes && exodus_timestep.size() != 1)
    paramError("exodus_timesteps_for_parameter_mesh_variable",
               "Number of timesteps to read mesh data specified by "
               "\"exodus_timesteps_for_parameter_mesh_variable\" incorrect. "
               "\"exodus_timesteps_for_parameter_mesh_variable\" can specify a single timestep or "
               "\"num_parameter_times\" timesteps.");

  _ndof = 0;
  for (const auto & param_id : make_range(_nparams))
  {
    // store off all the variable names that you might want to read from the mesh
    std::vector<std::string> var_names;
    if (isParamValid("initial_condition_mesh_variable"))
      var_names.push_back(initial_condition_mesh_variable[param_id]);
    if (isParamValid("lower_bound_mesh_variable"))
      var_names.push_back(lower_bound_mesh_variable[param_id]);
    if (isParamValid("upper_bound_mesh_variable"))
      var_names.push_back(upper_bound_mesh_variable[param_id]);

    const std::string family = families.size() > 1 ? families[param_id] : families[0];
    const std::string order = orders.size() > 1 ? orders[param_id] : orders[0];
    const FEType fetype(Utility::string_to_enum<Order>(order),
                        Utility::string_to_enum<FEFamily>(family));

    ParameterMesh pmesh(fetype, meshes[param_id], var_names);
    _nvalues[param_id] = pmesh.size() * ntimes;
    _ndof += _nvalues[param_id];

    // read and assign initial conditions
    std::vector<Real> initial_condition;
    if (isParamValid("initial_condition_mesh_variable"))
      initial_condition = parseExodusData(
          exodus_timestep, pmesh, initial_condition_mesh_variable[param_id], ntimes);
    else
      initial_condition = parseInputData("initial_condition", 0, param_id);

    _parameters[param_id]->assign(initial_condition.begin(), initial_condition.end());

    // read and assign lower bound
    std::vector<Real> lower_bound;
    if (isParamValid("lower_bound_mesh_variable"))
      lower_bound =
          parseExodusData(exodus_timestep, pmesh, lower_bound_mesh_variable[param_id], ntimes);
    else
      lower_bound = parseInputData("lower_bounds", std::numeric_limits<Real>::lowest(), param_id);

    _lower_bounds.insert(_lower_bounds.end(), lower_bound.begin(), lower_bound.end());

    // read and assign upper bound
    std::vector<Real> upper_bound;
    if (isParamValid("upper_bound_mesh_variable"))
      upper_bound =
          parseExodusData(exodus_timestep, pmesh, upper_bound_mesh_variable[param_id], ntimes);
    else
      upper_bound = parseInputData("upper_bounds", std::numeric_limits<Real>::max(), param_id);

    _upper_bounds.insert(_upper_bounds.end(), upper_bound.begin(), upper_bound.end());

    // resize gradient vector to be filled later
    _gradients[param_id]->resize(_nvalues[param_id]);
  }
}
