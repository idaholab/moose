//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ClosureTestAction.h"
#include "FlowModelSetup1Phase.h"

/**
 * Action for setting up a closure test for 1-phase flow
 */
class ClosureTest1PhaseAction : public ClosureTestAction, public FlowModelSetup1Phase
{
public:
  ClosureTest1PhaseAction(const InputParameters & params);

protected:
  virtual void addInitialConditions() override;
  virtual void addAuxVariables() override;
  virtual void addMaterials() override;
  virtual void addUserObjects() override;

public:
  static InputParameters validParams();
};
