//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectAction.h"

/**
 * MooseObjectAction for [NonlinearPreconditioning]/<name> sub-blocks.  Each sub-block creates one
 * NonlinearPreconditioning object; the sub-block name becomes the PETSc options prefix
 * for the outer SNES (e.g. -<name>_snes_type, -<name>_pc_type fieldsplit, etc.).
 */
class SetupNonlinearPreconditioningAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  SetupNonlinearPreconditioningAction(const InputParameters & params);

  virtual void act() override;
};
