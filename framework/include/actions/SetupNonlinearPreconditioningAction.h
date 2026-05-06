//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Action for the [NonlinearPreconditioning] input block.  Creates a NonlinearPreconditioning
 * object and stores it on the FEProblem so that FEProblemSolve can route inner-system solves
 * through the PETSc NPC mechanism.
 */
class SetupNonlinearPreconditioningAction : public Action
{
public:
  static InputParameters validParams();

  SetupNonlinearPreconditioningAction(const InputParameters & params);

  virtual void act() override;
};
