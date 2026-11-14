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
 * Holds the parameters of the [ConstructiveSolidGeometry] input block. The block is read by
 * MooseApp to enable CSG-only mode and by CSGOnlyAction to configure the CSG output; this action
 * itself does nothing.
 */
class CSGAction : public Action
{
public:
  static InputParameters validParams();

  CSGAction(const InputParameters & params);

  virtual void act() override;
};
