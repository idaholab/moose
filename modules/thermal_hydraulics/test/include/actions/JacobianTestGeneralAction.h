//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "JacobianTestAction.h"

/**
 * Action for setting up a Jacobian test that does not need physics setup
 *
 * This action sets up a Jacobian test that is agnostic of the physics used.
 * Instead of expecting particular solution variables, it takes a list of
 * the solution variables and their values.
 */
class JacobianTestGeneralAction : public JacobianTestAction
{
public:
  JacobianTestGeneralAction(const InputParameters & params);

protected:
  virtual void addInitialConditions() override;
  virtual void addSolutionVariables() override;
  virtual void addAuxVariables() override;
  virtual void addMaterials() override;
  virtual void addUserObjects() override;

  /// List of variables to add
  const std::vector<VariableName> _variables;

  /// List of values for the variables to add
  const std::vector<FunctionName> _variable_values;

public:
  static InputParameters validParams();
};
