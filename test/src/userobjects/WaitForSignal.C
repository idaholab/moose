//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WaitForSignal.h"
#include "Moose.h"

#include <thread>
#include <chrono>

registerMooseObject("MooseTestApp", WaitForSignal);

InputParameters
WaitForSignal::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Blocks the solve at a given time step until a Unix signal is "
                             "received, making signal-driven tests deterministic.");
  // Default to the first step: blocking here happens before the earliest
  // TIMESTEP_END at which the Checkpoint would consume the signal, so the
  // latch is still set when this object releases.
  params.addParam<unsigned int>(
      "wait_for_step", 1, "The time step at which to block until a signal is received");
  params.addParam<Real>(
      "timeout", 60, "Maximum number of seconds to wait before proceeding without a signal");
  // We need to block at the beginning of the step so the checkpoint, written at
  // the end of the step, captures the signal.
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;
  return params;
}

WaitForSignal::WaitForSignal(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _wait_for_step(getParam<unsigned int>("wait_for_step")),
    _timeout(getParam<Real>("timeout"))
{
}

void
WaitForSignal::execute()
{
  if (_t_step != static_cast<int>(_wait_for_step))
    return;

  _console << "Waiting for signal at time step " << _t_step << "." << std::endl;

  const auto start = std::chrono::steady_clock::now();
  bool timed_out = false;
  while (true)
  {
    // Sync the latched signal across processes, matching Output::outputStep so
    // that the checkpoint sees a consistent value at the end of the step.
    int signal_number = Moose::interrupt_signal_number;
    comm().max(signal_number);
    // Do not write back zero because the handler may have set the latch after the copy.
    if (signal_number)
      Moose::interrupt_signal_number = signal_number;

    const std::chrono::duration<Real> elapsed = std::chrono::steady_clock::now() - start;
    timed_out = elapsed.count() >= _timeout;

    // Decide collectively whether to release, so all ranks break on the same
    // iteration and none is left in a collective call by itself.
    int done = (signal_number != 0 || timed_out) ? 1 : 0;
    comm().max(done);
    if (done)
      break;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  if (timed_out && Moose::interrupt_signal_number == 0)
    mooseWarning("Timed out after ", _timeout, " s waiting for a signal; proceeding anyway.");
}
