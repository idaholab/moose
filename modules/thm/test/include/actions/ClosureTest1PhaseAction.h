#pragma once

#include "ClosureTestAction.h"
#include "FlowModelSetup1Phase.h"

class ClosureTest1PhaseAction;

template <>
InputParameters validParams<ClosureTest1PhaseAction>();

/**
 * Action for setting up a closure test for 1-phase flow
 */
class ClosureTest1PhaseAction : public ClosureTestAction, public FlowModelSetup1Phase
{
public:
  ClosureTest1PhaseAction(InputParameters params);

protected:
  virtual void addInitialConditions() override;
  virtual void addNonConstantAuxVariables() override;
  virtual void addMaterials() override;
  virtual void addUserObjects() override;
};
