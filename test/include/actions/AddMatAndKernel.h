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

/// This class creates a material-kernel with the kernel depending on the
/// material property.  This is meant to help diagnose/check for issues
/// relating to dynamically (in-code i.e. via actions) generated object
/// dependencies are handled correctly.
class AddMatAndKernel : public Action
{
public:
  static InputParameters validParams();

  AddMatAndKernel(const InputParameters & params);

  virtual void act();
};
