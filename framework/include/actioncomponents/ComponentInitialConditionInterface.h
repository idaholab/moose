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
 * Helper class to help Components define the initial conditions the Physics may need
 * from the parameters specified by the user
 * Note: Trying out virtual inheritance. It makes things
 *       a little easier to define as we can use the attributes
 *       of the underlying ActionComponent
 */
class ComponentInitialConditionInterface : public virtual ActionComponent
{
public:
  static InputParameters validParams();

  ComponentInitialConditionInterface(const InputParameters & params);

  /// Whether the component has an initial condition parameter set for the requested variable
  bool hasInitialCondition(const VariableName & variable) const;
  /// Get the name of the functor providing the initial condition for the requested variable
  const MooseFunctorName & getInitialCondition(const VariableName & variable,
                                               const std::string & requestor_name) const;

protected:
  virtual void checkIntegrity() override { checkInitialConditionsAllRequested(); }

  /// Names of the variables to set an initial condition on
  const std::vector<VariableName> _initial_condition_variables;
  /// Functor values for the initial conditions of the variables
  const std::vector<MooseFunctorName> _variable_ic_functors;
  /// Requested variables. If the IC for a variable was never requested, error
  mutable std::set<VariableName> _requested_ic_variables;

private:
  /// Checks that all initial conditions were requested.
  /// An unrequested property necessarily means an unused value
  void checkInitialConditionsAllRequested() const;
};
