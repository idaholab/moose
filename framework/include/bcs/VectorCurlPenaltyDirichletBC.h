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
 *  Enforces a Dirichlet boundary condition for the curl of vector nonlinear variables
 *  in a weak sense by applying a penalty to the difference in the current
 *  solution and the Dirichlet data.
 */
class VectorCurlPenaltyDirichletBC : public VectorIntegratedBC
{
public:
  static InputParameters validParams();

  VectorCurlPenaltyDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  /// The penalty coefficient
  Real _penalty;

  /// The vector function for the exact solution
  const Function * const _function;

  /// The function for the x component
  const Function & _function_x;

  /// The function for the y component
  const Function & _function_y;

  /// The function for the z component
  const Function & _function_z;
};
