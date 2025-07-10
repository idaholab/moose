//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "SetupInterface.h"
#include "PostprocessorInterface.h"
#include "PerfGraphInterface.h"
#include "TransientInterface.h"

/**
 * Base class for convergence criteria.
 */
class Convergence : public MooseObject,
                    public SetupInterface,
                    public PostprocessorInterface,
                    public PerfGraphInterface,
                    public TransientInterface
{
public:
  static InputParameters validParams();

  /**
   * Status returned by calls to \c checkConvergence.
   */
  enum class MooseConvergenceStatus
  {
    ITERATING = 0,
    CONVERGED = 1,
    DIVERGED = -1
  };

  /**
   * Iteration type
   */
  enum class IterationType
  {
    NONLINEAR = 0,
    LINEAR = 1, // solve_type = LINEAR, not linear solves within nonlinear solve
    MULTISYSTEM_FIXED_POINT = 2,
    MULTIAPP_FIXED_POINT = 3,
    STEADY = 4
  };

  Convergence(const InputParameters & parameters);

  virtual void initialSetup() override {}

  /// Perform checks related to the iteration type
  virtual void checkIterationType(IterationType /*it_type*/) const {}

  /**
   * Method that gets called before each iteration loop
   */
  virtual void initialize() {}

  /**
   * Method that gets called in each iteration before the solve
   */
  virtual void preExecute() {}

  /**
   * Returns convergence status.
   *
   * @param[in] iter   Iteration index
   */
  virtual MooseConvergenceStatus checkConvergence(unsigned int iter) = 0;

  /// Returns whether verbose mode has been enabled
  bool verbose() const { return _verbose; }

protected:
  /**
   * Outputs the stream to the console if verbose output is enabled
   */
  void verboseOutput(std::ostringstream & oss);

  /// Performance ID for \c checkConvergence
  PerfID _perfid_check_convergence;

  /// Thread ID
  THREAD_ID _tid;

private:
  /// Verbose mode enabled
  const bool _verbose;
};
