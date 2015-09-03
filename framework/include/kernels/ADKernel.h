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

#ifndef ADKERNEL_H
#define ADKERNEL_H

#include "KernelBase.h"

class ADKernel;

template<>
InputParameters validParams<ADKernel>();

class ADKernel :
  public KernelBase
{
public:
  ADKernel(const InputParameters & parameters);

  virtual ~ADKernel();

  // See KernelBase base for documentation of these overridden methods
  virtual void computeResidual();
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);
  virtual void computeOffDiagJacobianScalar(unsigned int jvar);

protected:
  /// Compute this Kernel's contribution to the residual at the current quadrature point
  virtual ADReal computeQpResidual() = 0;

  /// Holds the solution at current quadrature points
  ADVariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  ADVariableGradient & _grad_u;

  /// Time derivative of u
  VariableValue & _u_dot;

  /// Derivative of u_dot with respect to u
  VariableValue & _du_dot_du;
};

#endif /* ADKERNEL_H */
