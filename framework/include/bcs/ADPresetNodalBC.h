//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDirichletBCBase.h"

/**
 * Base class for automatic differentiation nodal BCs that (pre)set the solution
 * vector entries.
 *
 * Depcreated: inherit from ADDirichletBCBase instead and set the parameter
 * preset = true for the same behavior.
 */
class ADPresetNodalBC : public ADDirichletBCBase
{
public:
  ADPresetNodalBC(const InputParameters & parameters);

  static InputParameters validParams();
};
