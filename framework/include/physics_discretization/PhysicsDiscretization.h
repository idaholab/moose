//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"

/**
 * Discretization class to indicate how you plan to discretize the equation
 */
class PhysicsDiscretization : public MooseObject
{
public:
  static InputParameters validParams();

  PhysicsDiscretization(const InputParameters & parameters);

  /// Getter for the variables' names in the discretization
  std::vector<VariableName> getVariableNames() const { return _var_names; }
  /// Getter for the variables' families in the discretization
  std::vector<MooseEnum> getVariableFamilies() const { return _var_families; }
  /// Getter for the variables' orders in the discretization
  std::vector<MooseEnum> getVariableOrders() const { return _var_orders; }
  /// Getter for the variables' scaling in the discretization
  std::vector<Real> getVariableScalings() const { return _var_scalings; }

  void setVariableParams(const VariableName & name, InputParameters & params) const;

private:
  /// Vector of the variable names
  std::vector<VariableName> _var_names;
  /// Vector of the finite element families of the variables (same order as the names)
  std::vector<MooseEnum> _var_families;
  /// Vector of the orders of the variables (same order as the names)
  std::vector<MooseEnum> _var_orders;
  /// Vector of the scaling of the variables (same order as the names)
  std::vector<Real> _var_scalings;
};
