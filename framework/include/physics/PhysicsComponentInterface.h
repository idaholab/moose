//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsBase.h"
#include "MooseTypes.h"
#include "ComponentBoundaryConditionInterface.h"

/**
 * Interface class to help components interact with Physics
 */
class PhysicsComponentInterface : virtual public PhysicsBase
{
public:
  static InputParameters validParams();

  PhysicsComponentInterface(const InputParameters & parameters);

  /// Adds various info from the component.
  /// - subdomains (from PhysicsBase::addComponent)
  /// - initial conditions
  virtual void addComponent(const ActionComponent & component) override;

  /**
   * Add an initial condition from a component
   * @param component_name the name of the component to set the variable IC on
   * @param var_name the variable to provide the initial condition
   * @param ic_value the functor providing the initial condition
   */
  void addInitialCondition(const ComponentName & component_name,
                           const VariableName & var_name,
                           const MooseFunctorName & ic_value);

  /**
   * Add a boundary condition from a component
   * @param component_name the name of the component with the boundary condition
   * @param var_name the variable to provide the initial condition
   * @param boundary_name the name of the boundary condition
   * @param bc_value the functor providing the boundary condition
   * @param bc_type the type of the boundary condition, defined in
   * ComponentBoundaryConditionInterface at the moment
   */
  void
  addBoundaryCondition(const ComponentName & component_name,
                       const VariableName & var_name,
                       const BoundaryName & boundary_name,
                       const MooseFunctorName & bc_value,
                       const ComponentBoundaryConditionInterface::BoundaryConditionType & bc_type);

protected:
  // TODO: add hash constructor to ComponentName to be able to use it as a key
  /// Map of components to variables and initial conditions
  std::map<std::string, std::map<VariableName, MooseFunctorName>> _components_initial_conditions;
  /// Map of components to variables and boundary conditions
  std::map<std::string,
           std::map<std::pair<VariableName, BoundaryName>,
                    std::pair<MooseFunctorName,
                              ComponentBoundaryConditionInterface::BoundaryConditionType>>>
      _components_boundary_conditions;

private:
  virtual void actOnAdditionalTasks() override;

  // The default implementation of these routines will error if the Components have passed
  // information to the Physics, and the Physics has not implemented the expected behavior
  virtual void addInitialConditionsFromComponents();
  virtual void addBoundaryConditionsFromComponents();
};
