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
#ifndef EXCEPTIONKERNEL_H
#define EXCEPTIONKERNEL_H

#include "Kernel.h"

// Forward Declaration
class ExceptionKernel;

template <>
InputParameters validParams<ExceptionKernel>();

/**
 * Kernel that generates MooseException
 */
class ExceptionKernel : public Kernel
{
public:
  ExceptionKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  enum WhenType
  {
    RESIDUAL = 0,
    JACOBIAN,
    INITIAL_CONDITION
  } _when;

  /// True once the residual has thrown on any thread
  static bool _res_has_thrown;

  /// True once the Jacobian has thrown on any thread
  static bool _jac_has_thrown;

  /// Function which returns true if it's time to throw
  bool time_to_throw();
};

#endif /* EXCEPTIONKERNEL_H */
