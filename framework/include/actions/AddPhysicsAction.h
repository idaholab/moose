//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectAction.h"
#include "InputParameters.h"
#include "PhysicsBase.h"

/**
 * Build objects for a particular Physics specified by the [Physics] input file block
 * Physics can involve any MOOSE objects
 */
class AddPhysicsAction : public MooseObjectAction
{
public:
  static InputParameters validParams();
  AddPhysicsAction(const InputParameters & params);
  virtual void act() override;

private:
  /// Pointer to the physics to retrieve the other objects needed
  const PhysicsBase * _physics;
};
