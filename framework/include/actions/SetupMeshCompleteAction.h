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

class SetupMeshCompleteAction;

template <>
InputParameters validParams<SetupMeshCompleteAction>();

class SetupMeshCompleteAction : public Action
{
public:
  static InputParameters validParams();

  SetupMeshCompleteAction(InputParameters params);

  /**
   * Complete setup of the mesh
   * @param mesh The mesh for which to complete setup
   * @param safe_to_remove Whether it's safe to remove remote elements
   */
  bool completeSetup(MooseMesh * mesh, bool safe_to_remove = true);

  virtual void act() override;

  PerfID _uniform_refine_timer;
};
