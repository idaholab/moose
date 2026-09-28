//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeStepper.h"
#include "PostprocessorInterface.h"

#include "libmesh/system_norm.h"

/**
 * Computes time step size by applying PID control on the estimated local time-stepping error
 */
class PIDAdaptiveDT : public TimeStepper, public PostprocessorInterface
{
public:
  static InputParameters validParams();

  PIDAdaptiveDT(const InputParameters & parameters);

  virtual void acceptStep() override;

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;
  virtual bool constrainStep(Real & dt) override;

  /// Initial time step size
  const Real & _dt_initial;
  /// Option for evaluating the local error
  const int _error_estimation_option;
  /// True to let the code use the estimated initial error with the initial time step size as the local error tolerance
  const bool _auto_tolerance;
  /// Tolerance on the local error
  const Real & _local_error_tolerance;
  /// Variables used for evaluating the local error
  const std::vector<VariableName> _variables;
  /// The proportional gain (P)
  const Real & _proportional_gain;
  /// The integral gain (I)
  const Real & _integral_gain;
  /// The derivative gain (D)
  const Real & _derivative_gain;
  /// The maximum factor the time step size can grow with
  const Real & _max_increase_factor;
  /// The minimum factor the time step size can reduce with
  const Real & _min_decrease_factor;
  /// The maximum time step size set by a postprocessor
  const PostprocessorValue & _pp_dt_limiter;

private:
  /// Old time step size
  Real & _dt_old;
  /// Estimated local error
  Real & _error;
  /// Old estimated local error
  Real & _error_old;
  /// Older estimated local error
  Real & _error_older;
  /// Estimated initial local error
  Real & _initial_error;
  /// Specifying how to evaluate the local error
  std::unordered_map<const SystemBase *, libMesh::SystemNorm> _calc_norm;
};
