//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef IMPLICITODEX_H
#define IMPLICITODEX_H

#include "ODEKernel.h"

/**
 * The forward declaration is so that we can declare the validParams function
 * before we actually define the class... that way the definition isn't lost
 * at the bottom of the file.
 */

// Forward Declarations
class ImplicitODEx;

/**
 * validParams returns the parameters that this Kernel accepts / needs
 * The actual body of the function MUST be in the .C file.
 */
template <>
InputParameters validParams<ImplicitODEx>();

/**
 * ODE: x' = 3 * x + 2 * y
 */
class ImplicitODEx : public ODEKernel
{
public:
  /**
   * Constructor
   */
  ImplicitODEx(const InputParameters & parameters);

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
   *
   * Note that this can be an approximation or linearization.  In this case it's
   * not because the Jacobian of this operator is easy to calculate.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /**
   * Needed for computing off-diagonal terms in Jacobian
   */
  unsigned int _y_var;

  /**
   * Coupled scalar variable values
   */
  const VariableValue & _y;
};

#endif /* IMPLICITODEX_H */
