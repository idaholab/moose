//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBC.h"

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the value in the node
 */
class BndTestDirichletBC : public NodalBC
{
public:
  static InputParameters validParams();

  BndTestDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  /// The value for this BC
  const Real & _value;
};
