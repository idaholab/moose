//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActionComponent.h"
#include "InputParameters.h"
#include "MooseTypes.h"

/**
 * Helper class to help Components accept boundary condition parameters that the Physics may use
 * to generate the adequate boundary conditions
 * Note: Trying out virtual inheritance. It makes things
 *       a little easier to define as we can use the attributes
 *       of the underlying ActionComponent
 */
class ComponentBoundaryConditionInterface : public virtual ActionComponent
{
public:
  static InputParameters validParams();

  ComponentBoundaryConditionInterface(const InputParameters & params);

  enum BoundaryConditionType
  {
    FIXED_VALUE,
    FLUX
  };

  /**
   * Whether the component has a boundary condition parameter specified for the requested
   * variable
   * @param variable the name of the variable
   * @return Whether the component has a boundary condition parameter specified for the requested
   * variable
   */
  bool hasBoundaryCondition(const VariableName & variable) const;
  /**
   * Whether the component has a boundary condition parameter is set for the requested variable and
   * boundary
   * @param variable the name of the variable
   * @param boundary the name of the boundary
   * @return Whether the component has a boundary condition parameter specified
   */
  bool hasBoundaryCondition(const VariableName & variable, const BoundaryName & boundary) const;
  /**
   * Get the name of the boundaries on which the variable should have a boundary condition
   * @param variable the name of the variable concerned
   * @param requestor name of the requestor for the boundary condition, used in a potential error
   * message
   * @return the name of the boundaries
   */
  std::vector<BoundaryName> getBoundaryConditionBoundaries(const VariableName & variable) const;
  /**
   * Get the name of the functor providing the boundary condition for the requested variable and
   * boundary
   * @param variable the name of the variable concerned
   * @param boundary the name of the boundary concerned
   * @param requestor name of the requestor for the boundary condition, used in a potential error
   * message
   * @return bc_type the type of the boundary condition (flux or fixed value)
   * @return the name of the functor providing the value of the boundary condition
   */
  MooseFunctorName getBoundaryCondition(const VariableName & variable,
                                        const BoundaryName & boundary,
                                        const std::string & requestor_name,
                                        BoundaryConditionType & bc_type) const;

protected:
  virtual void checkIntegrity() override { checkBoundaryConditionsAllRequested(); }

  /// Names of the variables to set a fixed value BC on
  const std::vector<VariableName> _fixed_value_bc_variables;
  /// Names of the variables to set a flux BC on
  const std::vector<VariableName> _flux_bc_variables;

  // TODO: make our custom string types hashable
  // This would let us return the boundary condition functor name by reference too

  /// Maps of the fixed value boundary conditions
  std::map<std::string, std::map<std::string, std::string>> _fixed_value_bcs;
  /// Maps of the flux boundary conditions
  std::map<std::string, std::map<std::string, std::string>> _flux_bcs;

  /// Requested variables. If the IC for a variable was never requested, error
  mutable std::set<std::pair<VariableName, BoundaryName>> _requested_bc_variables;

private:
  /// Checks that all initial conditions were requested.
  /// An unrequested property necessarily means an unused value
  void checkBoundaryConditionsAllRequested() const;
};
