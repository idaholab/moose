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

#ifndef IMPLICITODEY_H
#define IMPLICITODEY_H

#include "ODEKernel.h"

/**
 * The forward declaration is so that we can declare the validParams function
 * before we actually define the class... that way the definition isn't lost
 * at the bottom of the file.
 */

// Forward Declarations
class ImplicitODEy;

/**
 * validParams returns the parameters that this Kernel accepts / needs
 * The actual body of the function MUST be in the .C file.
 */
template<>
InputParameters validParams<ImplicitODEy>();

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
  ImplicitODEy(const std::string & name, InputParameters parameters);

protected:
  /**
   * Responsible for computing the residual
   */
  virtual Real computeQpResidual();

  /**
   * Responsible for computing the diagonal block of the preconditioning matrix.
   * This is essentially the partial derivative of the residual with respect to
   * the variable this kernel operates on ("u").
   *
   * Note that this can be an approximation or linearization.  In this case it's
   * not because the Jacobian of this operator is easy to calculate.
   */
  virtual Real computeQpJacobian();

  /**
   * Responsible for computing the off-diagonal block of the preconditioning matrix.
   * This is essentially the partial derivative of the residual with respect to
   * the variable that is coupled into this kernel.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /**
   * Needed for computing off-diagonal terms in Jacobian
   */
  unsigned int _x_var;
  
  /**
   * Coupled scalar variable values
   */
  VariableValue & _x;
};


#endif /* IMPLICITODEY_H */
