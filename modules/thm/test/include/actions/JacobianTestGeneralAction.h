#pragma once

#include "JacobianTestAction.h"

class JacobianTestGeneralAction;

template <>
InputParameters validParams<JacobianTestGeneralAction>();

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
  JacobianTestGeneralAction(InputParameters params);

protected:
  virtual void addInitialConditions() override;
  virtual void addSolutionVariables() override;
  virtual void addNonConstantAuxVariables() override;
  virtual void addMaterials() override;
  virtual void addUserObjects() override;

  /// List of variables to add
  const std::vector<VariableName> _variables;

  /// List of values for the variables to add
  const std::vector<Real> _variable_values;
};
