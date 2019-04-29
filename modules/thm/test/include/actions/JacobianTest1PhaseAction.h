#pragma once

#include "JacobianTestAction.h"
#include "FlowModelSetup1Phase.h"

class JacobianTest1PhaseAction;

template <>
InputParameters validParams<JacobianTest1PhaseAction>();

/**
 * Action for setting up a Jacobian test for 1-phase flow
 */
class JacobianTest1PhaseAction : public JacobianTestAction, public FlowModelSetup1Phase
{
public:
  JacobianTest1PhaseAction(InputParameters params);

protected:
  virtual void addInitialConditions() override;
  virtual void addSolutionVariables() override;
  virtual void addNonConstantAuxVariables() override;
  virtual void addMaterials() override;
  virtual void addUserObjects() override;
};
