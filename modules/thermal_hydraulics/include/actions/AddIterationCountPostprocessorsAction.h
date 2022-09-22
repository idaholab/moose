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
 * Action that adds postprocessors for linear and nonlinear iterations
 */
class AddIterationCountPostprocessorsAction : public Action
{
public:
  AddIterationCountPostprocessorsAction(const InputParameters & parameters);

  virtual void act();

protected:
  /// True if iteration count postprocessors should be added
  bool _add_pps;

public:
  static InputParameters validParams();
};
