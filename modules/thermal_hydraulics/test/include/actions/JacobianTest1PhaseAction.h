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
#include "FlowModelSetup1Phase.h"

/**
 * Action for setting up a Jacobian test for 1-phase flow
 */
class JacobianTest1PhaseAction : public JacobianTestAction, public FlowModelSetup1Phase
{
public:
  JacobianTest1PhaseAction(const InputParameters & params);

protected:
  virtual void addInitialConditions() override;
  virtual void addSolutionVariables() override;
  virtual void addAuxVariables() override;
  virtual void addMaterials() override;
  virtual void addUserObjects() override;

public:
  static InputParameters validParams();
};
