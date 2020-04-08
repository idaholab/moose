//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DirichletBC.h"

class PresetBC;

template <>
InputParameters validParams<PresetBC>();

/**
 * Defines a boundary condition that (pre)sets the solution at the boundary
 * to be a user specified value.
 *
 * Deprecated: use DirichletBC with preset = true instead.
 */
class PresetBC : public DirichletBC
{
public:
  static InputParameters validParams();

  PresetBC(const InputParameters & parameters);

protected:
  virtual Real computeQpValue() override;
};
