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
 *  Enforces a Dirichlet boundary condition for the divergence of vector nonlinear
 *  variables in a weak sense by applying a penalty to the difference between the
 *  current solution and the Dirichlet data.
 */
class VectorDivPenaltyDirichletBC : public VectorIntegratedBC
{
public:
  static InputParameters validParams();

  VectorDivPenaltyDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

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
