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
 *  variables in a strong sense.
 */
class VectorDivDirichletBC : public VectorIntegratedBC
{
public:
  static InputParameters validParams();

  VectorDivDirichletBC(const InputParameters & parameters);

  virtual void computeResidual() override;
  /**
   * Computes d-ivar-residual / d-jvar...
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  /// The vector function for the exact solution
  const Function * const _function;

  /// The function for the x component
  const Function & _function_x;

  /// The function for the y component
  const Function & _function_y;

  /// The function for the z component
  const Function & _function_z;
};
