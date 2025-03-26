//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsComponentInterface.h"
#include "ComponentInitialConditionInterface.h"

InputParameters
PhysicsComponentInterface::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  // we likely do not need all these parameters. But developers could expect them
  return params;
}

PhysicsComponentInterface::PhysicsComponentInterface(const InputParameters & parameters)
  : PhysicsBase(parameters)
{
  addRequiredPhysicsTask("add_ic");
}

void
PhysicsComponentInterface::actOnAdditionalTasks()
{
  // TODO: find a way to force this routine to be called by derived classes
  if (_current_task == "add_ic")
    addInitialConditionsFromComponents();
  else if (_current_task == "add_bc" || _current_task == "add_fv_bc")
    addBoundaryConditionsFromComponents();
}

void
PhysicsComponentInterface::addComponent(const ActionComponent & comp)
{
  // Adds the component's blocks to the block restriction at least
  PhysicsBase::addComponent(comp);

  // Move initial conditions from components to the Physics
  if (const auto comp_ic = dynamic_cast<const ComponentInitialConditionInterface *>(&comp))
  {
    for (const auto & var_name : solverVariableNames())
      if (comp_ic->hasInitialCondition(var_name))
        addInitialCondition(comp.name(), var_name, comp_ic->getInitialCondition(var_name, name()));
    for (const auto & var_name : auxVariableNames())
      if (comp_ic->hasInitialCondition(var_name))
        addInitialCondition(comp.name(), var_name, comp_ic->getInitialCondition(var_name, name()));
  }

  // Move boundary conditions from components to the Physics
  if (const auto comp_bc = dynamic_cast<const ComponentBoundaryConditionInterface *>(&comp))
  {
    for (const auto & var_name : solverVariableNames())
      if (comp_bc->hasBoundaryCondition(var_name))
        for (const auto & boundary : comp_bc->getBoundaryConditionBoundaries(var_name))
        {
          ComponentBoundaryConditionInterface::BoundaryConditionType bc_type;
          const auto boundary_functor =
              comp_bc->getBoundaryCondition(var_name, boundary, name(), bc_type);
          addBoundaryCondition(comp.name(), var_name, boundary, boundary_functor, bc_type);
        }
    for (const auto & var_name : auxVariableNames())
      if (comp_bc->hasBoundaryCondition(var_name))
        for (const auto & boundary : comp_bc->getBoundaryConditionBoundaries(var_name))
        {
          ComponentBoundaryConditionInterface::BoundaryConditionType bc_type;
          const auto boundary_functor =
              comp_bc->getBoundaryCondition(var_name, boundary, name(), bc_type);
          addBoundaryCondition(comp.name(), var_name, boundary, boundary_functor, bc_type);
        }
  }
}

void
PhysicsComponentInterface::addInitialCondition(const ComponentName & component_name,
                                               const VariableName & var_name,
                                               const MooseFunctorName & ic_value)
{
  _components_initial_conditions[component_name][var_name] = ic_value;
}

void
PhysicsComponentInterface::addBoundaryCondition(
    const ComponentName & component_name,
    const VariableName & var_name,
    const BoundaryName & boundary_name,
    const MooseFunctorName & bc_value,
    const ComponentBoundaryConditionInterface::BoundaryConditionType & bc_type)
{
  _components_boundary_conditions[component_name][std::make_pair(var_name, boundary_name)] =
      std::make_pair(bc_value, bc_type);
}

void
PhysicsComponentInterface::addInitialConditionsFromComponents()
{
  if (_components_initial_conditions.size())
  {
    std::vector<ComponentName> all_components(_components_initial_conditions.size());
    for (const auto & comp : _components_initial_conditions)
      all_components.push_back(comp.first);
    mooseError("Component(s) '",
               Moose::stringify(all_components),
               "' requested to add the following initial conditions for this Physics. This Physics "
               "however does not implement the 'addInitialConditionsFromComponents' "
               "routine, so it cannot create these initial conditions");
  }
}

void
PhysicsComponentInterface::addBoundaryConditionsFromComponents()
{
  if (_components_boundary_conditions.size())
  {
    std::vector<ComponentName> all_components(_components_boundary_conditions.size());
    for (const auto & comp : _components_boundary_conditions)
      all_components.push_back(comp.first);
    mooseError("Component(s) '",
               Moose::stringify(all_components),
               "' requested to add boundary conditions for the variable of this Physics. This "
               "Physics however does not implement the 'addBoundaryConditionsFromComponents' "
               "routine, so it cannot create these boundary conditions");
  }
}
