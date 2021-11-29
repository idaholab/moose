//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ODEKernel.h"

/**
 * Kernel that implements the ODE for y-variable
 *
 * ODE: y' = 4 * x + y
 */
class ImplicitODEy : public ODEKernel
{
public:
  /**
   * Constructor
   */
  ImplicitODEy(const InputParameters & parameters);

  /**
   * validParams returns the parameters that this Kernel accepts / needs
   * The actual body of the function MUST be in the .C file.
   */
  static InputParameters validParams();

protected:
  /**
   * Responsible for computing the residual
   */
  virtual Real computeQpResidual() override;

  /**
   * Responsible for computing the diagonal block of the preconditioning matrix.
   * This is essentially the partial derivative of the residual with respect to
   * the variable this kernel operates on ("u").
   *
   * Note that this can be an approximation or linearization.  In this case it's
   * not because the Jacobian of this operator is easy to calculate.
   */
  virtual Real computeQpJacobian() override;

  /**
   * Responsible for computing the off-diagonal block of the preconditioning matrix.
   * This is essentially the partial derivative of the residual with respect to
   * the variable that is coupled into this kernel.
   */
  virtual Real computeQpOffDiagJacobianScalar(unsigned int jvar) override;

  /**
   * Needed for computing off-diagonal terms in Jacobian
   */
  const unsigned int _x_var;

  /**
   * Coupled scalar variable values
   */
  const VariableValue & _x;
};
