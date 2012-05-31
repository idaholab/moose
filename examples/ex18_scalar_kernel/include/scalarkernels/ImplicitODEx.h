/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef IMPLICITODEX_H
#define IMPLICITODEX_H


#include "ODEKernel.h"

/**
 * The forward declaration is so that we can declare the validParams function
 * before we actually define the class... that way the definition isn't lost
 * at the bottom of the file.
 */

//Forward Declarations
class ImplicitODEx;

/**
 * validParams returns the parameters that this Kernel accepts / needs
 * The actual body of the function MUST be in the .C file.
 */
template<>
InputParameters validParams<ImplicitODEx>();

/**
 * ODE: x' = 3 * x + 2 * y
 *
 */
class ImplicitODEx : public ODEKernel
{
public:
  /**
   * This is the Constructor declaration AND definition.
   * It is ok to have the definition in the .h if the function body
   * is really small.  Otherwise it should be in the .C
   */
  ImplicitODEx(const std::string & name, InputParameters parameters);

protected:
  /**
   * Responsible for computing the residual
   *
   * This should always be defined in the .C
   */
  virtual Real computeQpResidual();

  /**
   * Responsible for computing the diagonal block of the preconditioning matrix.
   * This is essentially the partial derivative of the residual with respect to
   * the variable this kernel operates on ("u").
   *
   * Note that this can be an approximation or linearization.  In this case it's
   * not because the Jacobian of this operator is easy to calculate.
   *
   * This should always be defined in the .C
   */
  virtual Real computeQpJacobian();

  /**
   * Responsible for computing the off-diagonal block of the preconditioning matrix.
   * This is essentially the partial derivative of the residual with respect to
   * the variable that is coupled into this kernel.
   *
   * Note that this can be an approximation or linearization.  In this case it's
   * not because the Jacobian of this operator is easy to calculate.
   *
   * This should always be defined in the .C
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// The number of coupled scalar variable
  /// Needed for computing off-diagonal terms in Jacobian
  unsigned int _y_var;
  /// Coupled scalar variable values
  VariableValue & _y;
};


#endif /* IMPLICITODEX_H */
