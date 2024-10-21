//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Action for creating the actions triggered for the [Mesh] block
 * when the [Mesh] block is not present, but the [ActionComponents] block is
 */
class CreateMeshSetupActionsForComponents : public Action
{
public:
  static InputParameters validParams();

  CreateMeshSetupActionsForComponents(const InputParameters & params);

  virtual void act() override;
};
