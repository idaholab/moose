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

template<>
InputParameters validParams<ExceptionKernel>();

/**
 * Kernel that generates MooseException
 */
class ExceptionKernel : public Kernel
{
public:
  ExceptionKernel(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  enum WhenType {
    RESIDUAL = 0,
    JACOBIAN,
    INITIAL_CONDITION
  } _when;

  /// Counter for the number of computeQpResidual calls
  unsigned int _call_no;

  /// Counter for the number of computeQpJacobian calls
  unsigned int _jac_call_no;
};

#endif /* EXCEPTIONKERNEL_H */
