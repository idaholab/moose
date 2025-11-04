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
 * Adds tagged matrices. This is its own action as opposed to being performed at problem
 * construction time because other objects, such as preconditioners, may influence the underlying
 * types of the added matrices
 */
class AddTaggedMatricesAction : public Action
{
public:
  static InputParameters validParams();

  AddTaggedMatricesAction(const InputParameters & params);

  virtual void act() override;
};
