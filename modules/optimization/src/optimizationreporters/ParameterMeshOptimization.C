//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParameterMeshOptimization.h"

#include "AddVariableAction.h"
#include "ParameterMesh.h"
#include "OptUtils.h"
#include "libmesh/string_to_enum.h"

#include "ReadExodusMeshVars.h"

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

  // New parameters for multiple regularization types
  MultiMooseEnum reg_types("L2_GRADIENT");
  params.addParam<MultiMooseEnum>(
      "regularization_types",
      reg_types,
      "Types of regularization to apply. Multiple types can be specified.");

  params.addParam<std::vector<Real>>("regularization_coeffs",
                                     {},
                                     "Coefficients for each regularization type. Must match the "
                                     "number of regularization_types specified.");

  params.addParamNamesToGroup("tikhonov_coeff regularization_types regularization_coeffs",
                              "Regularization");

  return params;
}

ParameterMeshOptimization::ParameterMeshOptimization(const InputParameters & parameters)
  : GeneralOptimization(parameters),
    _regularization_coeffs(getParam<std::vector<Real>>("regularization_coeffs")),
    _regularization_types(getParam<MultiMooseEnum>("regularization_types")
                              .getSetValueIDs<ParameterMesh::RegularizationType>())
{
  // Validate that regularization coefficients match types
  if (_regularization_coeffs.size() != _regularization_types.size())
    paramError("regularization_coeffs",
               "Number of regularization coefficients (",
               _regularization_coeffs.size(),
               ") must match number of regularization types (",
               _regularization_types.size(),
               ")");
}

std::vector<Real>
ParameterMeshOptimization::parseExodusData(const FEType fetype,
                                           const FileName mesh_file_name,
                                           const std::vector<unsigned int> & exodus_timestep,
                                           const std::string & mesh_var_name) const
{
  // read data off Exodus mesh
  ReadExodusMeshVars data_mesh(fetype, mesh_file_name, mesh_var_name);
  std::vector<Real> parsed_data;
  // read from mesh
  for (auto const & step : exodus_timestep)
  {
    std::vector<Real> data = data_mesh.getParameterValues(step);
    parsed_data.insert(parsed_data.end(), data.begin(), data.end());
  }

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
  _parameter_meshes.resize(_nparams);
  for (const auto & param_id : make_range(_nparams))
  {
    const std::string family = families.size() > 1 ? families[param_id] : families[0];
    const std::string order = orders.size() > 1 ? orders[param_id] : orders[0];
    const FEType fetype(Utility::string_to_enum<Order>(order),
                        Utility::string_to_enum<FEFamily>(family));

    _parameter_meshes[param_id] = std::make_unique<ParameterMesh>(fetype, meshes[param_id]);
    _nvalues[param_id] = _parameter_meshes[param_id]->size() * ntimes;
    _ndof += _nvalues[param_id];

    // read and assign initial conditions
    {
      std::vector<Real> initial_condition;
      if (isParamValid("initial_condition_mesh_variable"))
        initial_condition = parseExodusData(
            fetype, meshes[param_id], exodus_timestep, initial_condition_mesh_variable[param_id]);
      else
        initial_condition = parseInputData("initial_condition", 0, param_id);

      _parameters[param_id]->assign(initial_condition.begin(), initial_condition.end());
    }

    // read and assign lower bound
    {
      std::vector<Real> lower_bound;
      if (isParamValid("lower_bound_mesh_variable"))
        lower_bound = parseExodusData(
            fetype, meshes[param_id], exodus_timestep, lower_bound_mesh_variable[param_id]);
      else
        lower_bound = parseInputData("lower_bounds", std::numeric_limits<Real>::lowest(), param_id);

      _lower_bounds.insert(_lower_bounds.end(), lower_bound.begin(), lower_bound.end());
    }

    // read and assign upper bound
    {
      std::vector<Real> upper_bound;
      if (isParamValid("upper_bound_mesh_variable"))
        upper_bound = parseExodusData(
            fetype, meshes[param_id], exodus_timestep, upper_bound_mesh_variable[param_id]);
      else
        upper_bound = parseInputData("upper_bounds", std::numeric_limits<Real>::max(), param_id);

      _upper_bounds.insert(_upper_bounds.end(), upper_bound.begin(), upper_bound.end());
    }

    // resize gradient vector to be filled later
    _gradients[param_id]->resize(_nvalues[param_id]);
  }
}

Real
ParameterMeshOptimization::computeObjective()
{
  Real val = GeneralOptimization::computeObjective();

  // Apply each regularization type with its coefficient
  for (const auto reg_idx : index_range(_regularization_types))
  {
    if (_regularization_coeffs[reg_idx] > 0.0)
    {
      Real regularization_value = 0.0;

      // Convert MultiMooseEnum to RegularizationType using get() method
      ParameterMesh::RegularizationType reg_type = _regularization_types[reg_idx];

      for (const auto & param_id : make_range(_nparams))
      {
        // Get current parameter values for this group
        const auto & param_values = *_parameters[param_id];

        // Compute regularization objective for this type
        regularization_value +=
            _parameter_meshes[param_id]->computeRegularizationObjective(param_values, reg_type);
      }

      val += _regularization_coeffs[reg_idx] * regularization_value;
    }
  }

  return val;
}

void
ParameterMeshOptimization::computeGradient(libMesh::PetscVector<Number> & gradient) const
{
  // Add regularization gradient contributions to the reporter gradients before base computation
  for (const auto reg_idx : index_range(_regularization_types))
  {
    if (_regularization_coeffs[reg_idx] > 0.0)
    {
      // Convert MultiMooseEnum to RegularizationType using get() method
      ParameterMesh::RegularizationType reg_type = _regularization_types[reg_idx];

      for (const auto & param_id : make_range(_nparams))
      {
        // Get current parameter values for this group
        const auto & param_values = *_parameters[param_id];
        auto grad_values = _gradients[param_id];

        // Compute regularization gradient for this type
        std::vector<Real> reg_grad =
            _parameter_meshes[param_id]->computeRegularizationGradient(param_values, reg_type);

        // Add to gradient with coefficient
        for (unsigned int i = 0; i < param_values.size(); ++i)
          (*grad_values)[i] += _regularization_coeffs[reg_idx] * reg_grad[i];
      }
    }
  }

  // Now call base class method which includes Tikhonov and copies to PETSc vector
  OptimizationReporterBase::computeGradient(gradient);
}
