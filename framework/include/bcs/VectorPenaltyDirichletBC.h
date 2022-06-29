//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorIntegratedBC.h"

/**
 *  Enforces a Dirichlet boundary condition for vector nonlinear variables in a
 *  weak sense by applying a penalty to the difference in the current solution and
 *  the Dirichlet data.
 */
class VectorPenaltyDirichletBC : public VectorIntegratedBC
{
public:
  static InputParameters validParams();

  VectorPenaltyDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  /// The penalty coefficient
  Real _penalty;

  /// The exact solution for the x component
  const Function & _exact_x;

  /// The exact solution for the y component
  const Function & _exact_y;

  /// The exact solution for the z component
  const Function & _exact_z;

  /// Whether this is formulated as a linear problem
  const bool _linear;
};
