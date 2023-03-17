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
 * Automatically generates all the L variables for the RFF phase field crystal model.
 */
class HHPFCRFFSplitVariablesAction : public Action
{
public:
  static InputParameters validParams();

  HHPFCRFFSplitVariablesAction(const InputParameters & params);

  virtual void act();

private:
  const unsigned int _num_L;
  const std::string _L_name_base;
};
