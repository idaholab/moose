//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionDirichletBC.h"

// Forward Declarations
class FunctionPresetBC;

template <>
InputParameters validParams<FunctionPresetBC>();

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 *
 * Deprecated: use FunctionDirichletBC with preset = true instead.
 */
class FunctionPresetBC : public FunctionDirichletBC
{
public:
  static InputParameters validParams();

  FunctionPresetBC(const InputParameters & parameters);
};
