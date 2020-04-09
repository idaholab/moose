//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "Action.h"

/**
 * Automatically generates all variables and kernels to set up a KKS phase field simulation
 */
class KKSAction : public Action
{
public:
  static InputParameters validParams();

  KKSAction(const InputParameters & params);
  virtual void act();

private:
  std::string _c_name_base;
};
