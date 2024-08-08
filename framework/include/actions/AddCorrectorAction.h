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
 * Action for creating Corrector, a UserObject
 * designed for nonlinear variable solutions without constraints.
 */
class AddCorrectorAction : public MooseObjectAction
{
public:
  /**
   * Class constructor
   * @param params Parameters for this Action
   */
  static InputParameters validParams();

  AddCorrectorAction(const InputParameters & params);

  virtual void act() override;
};
