//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executioner.h"
#include "SIMPLESolveBase.h"

// Forward declarations
class InputParameters;
class FEProblemBase;

/**
 * Base class for the executioners relying on segregated solution approaches.
 */
class SegregatedSolverBase : public Executioner
{
public:
  static InputParameters validParams();

  SegregatedSolverBase(const InputParameters & parameters);

  virtual void init() override;
  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

protected:
  /// Check for wrong execute on flags in the multiapps
  bool hasMultiAppError(const ExecFlagEnum & flags);

  /// Check for wrong execute on flags in the transfers
  bool hasTransferError(const ExecFlagEnum & flags);

  /// Reference to the MOOSE Problem which contains this executioner
  FEProblemBase & _problem;

  /// Time-related member variables. Only used to set the steady-state result
  /// at time 1.0;
  Real _system_time;
  int & _time_step;
  Real & _time;

  bool _last_solve_converged;
};
