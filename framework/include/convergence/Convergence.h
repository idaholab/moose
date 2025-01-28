//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

  Convergence(const InputParameters & parameters);

  virtual void initialSetup() override{};

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
