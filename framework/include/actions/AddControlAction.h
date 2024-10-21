//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseObjectAction.h"

/**
 * Action for creating Control objects
 *
 * Control objects are GeneralUserObjects, thus just
 * use the AddUserObjectAction
 */
class AddControlAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddControlAction(const InputParameters & parameters);

  virtual void act() override;
};
