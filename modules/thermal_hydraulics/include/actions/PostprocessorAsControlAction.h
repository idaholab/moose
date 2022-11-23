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

/**
 * This action creates a control value named the same as the postprocessor being added
 *
 * This allows people to use the postprocessor values directly within the control system.
 */
class PostprocessorAsControlAction : public MooseObjectAction
{
public:
  PostprocessorAsControlAction(const InputParameters & params);

  virtual void act();

public:
  static InputParameters validParams();
};
