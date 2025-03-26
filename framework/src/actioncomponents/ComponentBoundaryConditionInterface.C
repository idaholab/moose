// //* This file is part of the MOOSE framework
// //* https://mooseframework.inl.gov
// //*
// //* All rights reserved, see COPYRIGHT for full restrictions
// //* https://github.com/idaholab/moose/blob/master/COPYRIGHT
// //*
// //* Licensed under LGPL 2.1, please see LICENSE for details
// //* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComponentBoundaryConditionInterface.h"

InputParameters
ComponentBoundaryConditionInterface::validParams()
{
  auto params = ActionComponent::validParams();

  // Support two common types of boundary conditions
  params.addParam<std::vector<VariableName>>(
      "fixed_value_bc_variables",
      {},
      "List of variables that have fixed value boundary condition(s) defined on this component");
  params.addParam<std::vector<std::vector<BoundaryName>>>(
      "fixed_value_bc_boundaries",
      {},
      "Boundaries on which to apply the fixed value boundary condition(s). Outer ordering is "
      "variables, inner order is surfaces");
  params.addParam<std::vector<std::vector<MooseFunctorName>>>(
      "fixed_value_bc_values",
      {},
      "Functors that provide the fixed value boundary condition(s) values. Outer ordering is "
      "variables, inner order is surfaces");

  params.addParam<std::vector<VariableName>>(
      "flux_bc_variables",
      {},
      "List of variables that have flux boundary condition(s) defined on this component");
  params.addParam<std::vector<std::vector<BoundaryName>>>(
      "flux_bc_boundaries",
      {},
      "Boundaries on which to apply the flux boundary condition(s). Outer ordering is "
      "variables, inner order is surfaces");
  params.addParam<std::vector<std::vector<MooseFunctorName>>>(
      "flux_bc_values",
      {},
      "Functors that provide the flux boundary condition(s) values. Outer ordering is "
      "variables, inner order is surfaces");

  params.addParamNamesToGroup(
      "fixed_value_bc_variables fixed_value_bc_boundaries fixed_value_bc_values flux_bc_variables "
      " flux_bc_boundaries flux_bc_values",
      "Variable boundary conditions");
  return params;
}

ComponentBoundaryConditionInterface::ComponentBoundaryConditionInterface(
    const InputParameters & params)
  : ActionComponent(params),
    _fixed_value_bc_variables(getParam<std::vector<VariableName>>("fixed_value_bc_variables")),
    _flux_bc_variables(getParam<std::vector<VariableName>>("flux_bc_variables"))
{
  addRequiredTask("init_component_physics");
  addRequiredTask("check_integrity");

  // Check for unique elements
  checkVectorParamsNoOverlap<VariableName>({"fixed_value_bc_variables"});
  checkVectorParamsNoOverlap<VariableName>({"flux_bc_variables"});

  // Check parameter sizes
  const auto & fixed_value_boundaries =
      getParam<std::vector<std::vector<BoundaryName>>>("fixed_value_bc_boundaries");
  const auto & fixed_value_values =
      getParam<std::vector<std::vector<MooseFunctorName>>>("fixed_value_bc_values");
  checkVectorParamsSameLength<VariableName, std::vector<BoundaryName>>("fixed_value_bc_variables",
                                                                       "fixed_value_bc_boundaries");
  checkVectorParamsSameLength<VariableName, std::vector<MooseFunctorName>>(
      "fixed_value_bc_variables", "fixed_value_bc_values");
  checkTwoDVectorParamsSameLength<BoundaryName, MooseFunctorName>("fixed_value_bc_boundaries",
                                                                  "fixed_value_bc_values");
  const auto & flux_boundaries =
      getParam<std::vector<std::vector<BoundaryName>>>("flux_bc_boundaries");
  const auto & flux_values = getParam<std::vector<std::vector<MooseFunctorName>>>("flux_bc_values");
  checkVectorParamsSameLength<VariableName, std::vector<BoundaryName>>("flux_bc_variables",
                                                                       "flux_bc_boundaries");
  checkVectorParamsSameLength<VariableName, std::vector<MooseFunctorName>>("flux_bc_variables",
                                                                           "flux_bc_values");
  checkTwoDVectorParamsSameLength<BoundaryName, MooseFunctorName>("flux_bc_boundaries",
                                                                  "flux_bc_values");

  // NOTE: We could add a check that for a (variable,boundary) pair, we do not have a fixed value
  // and a flux BC defined at the same time
  // TODO: add another check that the boundaries are part of the component
  // This is not possible yet, components do not keep the list of their boundaries

  // Form maps for the variable-boundary-value from the input vectors
  for (const auto i : index_range(_fixed_value_bc_variables))
  {
    const auto & var_name = _fixed_value_bc_variables[i];
    for (const auto j : index_range(fixed_value_boundaries[i]))
      _fixed_value_bcs[var_name][fixed_value_boundaries[i][j]] = fixed_value_values[i][j];
  }
  for (const auto i : index_range(_flux_bc_variables))
  {
    const auto & var_name = _flux_bc_variables[i];
    for (const auto j : index_range(flux_boundaries[i]))
      _flux_bcs[var_name][flux_boundaries[i][j]] = flux_values[i][j];
  }
}

bool
ComponentBoundaryConditionInterface::hasBoundaryCondition(const VariableName & var_name) const
{
  // if there exists at least one BC of that type for that variable
  const bool has_fixed_value = _fixed_value_bcs.count(var_name);
  const bool has_flux = _flux_bcs.count(var_name);

  return has_fixed_value || has_flux;
}

bool
ComponentBoundaryConditionInterface::hasBoundaryCondition(const VariableName & var_name,
                                                          const BoundaryName & boundary) const
{
  // if there exists at least one BC of that type for that variable
  const bool has_fixed_value = _fixed_value_bcs.count(var_name);
  const bool has_flux = _flux_bcs.count(var_name);

  // Now check for that specific boundary
  if (has_fixed_value && libmesh_map_find(_fixed_value_bcs, var_name).count(boundary))
    return true;
  else if (has_flux && libmesh_map_find(_flux_bcs, var_name).count(boundary))
    return true;
  else
    return false;
}

MooseFunctorName
ComponentBoundaryConditionInterface::getBoundaryCondition(const VariableName & var_name,
                                                          const BoundaryName & boundary,
                                                          const std::string & requestor_name,
                                                          BoundaryConditionType & bc_type) const
{
  // Ideally all boundary conditions defined by the user in the input will get requested
  _requested_bc_variables.insert(std::make_pair(var_name, boundary));

  // if there exists at least one BC of that type for that variable
  const bool has_fixed_value = _fixed_value_bcs.count(var_name);
  const bool has_flux = _flux_bcs.count(var_name);

  // Now check for that specific boundary
  if (has_fixed_value && libmesh_map_find(_fixed_value_bcs, var_name).count(boundary))
  {
    bc_type = FIXED_VALUE;
    return libmesh_map_find(libmesh_map_find(_fixed_value_bcs, var_name), boundary);
  }
  else if (has_flux && libmesh_map_find(_flux_bcs, var_name).count(boundary))
  {
    bc_type = FLUX;
    return libmesh_map_find(libmesh_map_find(_flux_bcs, var_name), boundary);
  }
  else
    paramError("fixed_value_bc_variables",
               "Boundary condition for variable '" + var_name + "' on boundary '" + boundary +
                   "' requested by '" + requestor_name +
                   "' has not been specified on this ActionComponent.");
}

std::vector<BoundaryName>
ComponentBoundaryConditionInterface::getBoundaryConditionBoundaries(
    const VariableName & var_name) const
{
  // if there exists at least one BC of that type for that variable
  const bool has_fixed_value = _fixed_value_bcs.count(var_name);
  const bool has_flux = _flux_bcs.count(var_name);

  std::vector<BoundaryName> boundaries;

  if (has_fixed_value)
    for (const auto & boundary_pair : libmesh_map_find(_fixed_value_bcs, var_name))
      boundaries.push_back(boundary_pair.first);

  if (has_flux)
    for (const auto & boundary_pair : libmesh_map_find(_flux_bcs, var_name))
      boundaries.push_back(boundary_pair.first);

  return boundaries;
}

void
ComponentBoundaryConditionInterface::checkBoundaryConditionsAllRequested() const
{
  std::string list_missing = "";

  for (const auto & var_pair : _fixed_value_bcs)
    for (const auto & bc_pair : var_pair.second)
      if (std::find(_requested_bc_variables.begin(),
                    _requested_bc_variables.end(),
                    std::make_pair(VariableName(var_pair.first), BoundaryName(bc_pair.first))) ==
          _requested_bc_variables.end())
        list_missing += "\n- " + var_pair.first + " on " + bc_pair.first;

  for (const auto & var_pair : _flux_bcs)
    for (const auto & bc_pair : var_pair.second)
      if (std::find(_requested_bc_variables.begin(),
                    _requested_bc_variables.end(),
                    std::make_pair(VariableName(var_pair.first), BoundaryName(bc_pair.first))) ==
          _requested_bc_variables.end())
        list_missing += "\n- " + var_pair.first + " on " + bc_pair.first;

  if (!list_missing.empty())
    mooseError("Boundary conditions for variables and boundaries:" + list_missing +
               "\n  have been defined on this ActionComponent, but have not been requested by "
               "any Physics.");
}
