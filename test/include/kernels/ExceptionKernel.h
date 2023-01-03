//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * Kernel that generates MooseException
 */
class ExceptionKernel : public Kernel
{
public:
  static InputParameters validParams();

  ExceptionKernel(const InputParameters & parameters);

  virtual void jacobianSetup() override;
  virtual void residualSetup() override;

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

  // Determine the type of exception to throw (something not normally caught versus normally caught)
  const bool _throw_std_exception;

  // The rank to isolate the exception to if valid
  const processor_id_type _rank;

  /// True once the residual has thrown on any thread
  static bool _res_has_thrown;

  /// True once the Jacobian has thrown on any thread
  static bool _jac_has_thrown;

  /// Function which returns true if it's time to throw
  bool time_to_throw() const;

  /// count down to throwing
  int _counter;
};
