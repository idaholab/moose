//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Initialize components
 */
class THMInitComponentsAction : public Action
{
public:
  THMInitComponentsAction(const InputParameters & parameters);

  virtual void act();

public:
  static InputParameters validParams();
};
