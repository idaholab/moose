//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectAction.h"

/**
 * Builds the Remesher of the [Remeshing/Remeshers] block and hands it to the remeshing engine.
 */
class AddRemesherAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddRemesherAction(const InputParameters & params);

  virtual void act() override;
};
