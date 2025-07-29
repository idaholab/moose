//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosObjectAction.h"

/**
 * Adds a Kokkos material for GPU computation
 * Associated with the [KokkosMaterials] syntax
 */
class AddKokkosMaterialAction : public KokkosObjectAction
{
public:
  static InputParameters validParams();

  AddKokkosMaterialAction(const InputParameters & params);

  virtual void act() override;
};
