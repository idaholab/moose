//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * Test object that parks the solve at a deterministic time step until a Unix
 * signal (e.g. SIGUSR1) has been received. This removes the race in the signal
 * handler tests, where a fast solve could finish before the test harness
 * managed to deliver the signal.
 */
class WaitForSignal : public GeneralUserObject
{
public:
  static InputParameters validParams();

  WaitForSignal(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

private:
  /// The time step at which to block until a signal is received
  const unsigned int _wait_for_step;

  /// Maximum number of seconds to wait before giving up and proceeding anyway
  const Real _timeout;
};
