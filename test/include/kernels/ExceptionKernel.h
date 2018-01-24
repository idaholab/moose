//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  enum class WhenType
  {
    RESIDUAL = 0,
    JACOBIAN,
    INITIAL_CONDITION
  } _when;

  // Determine whether we should throw an exception or just trigger an error (abort)
  const bool _should_throw;

  // The rank to isolate the exception to if valid
  const processor_id_type _rank;

  /// True once the residual has thrown on any thread
  static bool _res_has_thrown;

  /// True once the Jacobian has thrown on any thread
  static bool _jac_has_thrown;

  /// Function which returns true if it's time to throw
  bool time_to_throw() const;
};

#endif /* EXCEPTIONKERNEL_H */
