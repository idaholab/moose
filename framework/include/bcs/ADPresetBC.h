//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDirichletBC.h"

/**
 * Defines a boundary condition that (pre)sets the solution at the boundary
 * to be a user specified value.
 */
class ADPresetBC : public ADDirichletBC
{
public:
  static InputParameters validParams();

  ADPresetBC(const InputParameters & parameters);
};
