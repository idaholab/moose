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

#include "TimeSequenceStepperBase.h"
#include "FEProblem.h"
#include "Transient.h"

template<>
InputParameters validParams<TimeSequenceStepperBase>()
{
  InputParameters params = validParams<TimeStepper>();
  return params;
}

TimeSequenceStepperBase::TimeSequenceStepperBase(const InputParameters & parameters) :
    TimeStepper(parameters),
    _current_step(declareRestartableData("current_step", (unsigned int) 0)),
    _time_sequence(declareRestartableData<std::vector<Real> >("time_sequence"))
{
}

void
TimeSequenceStepperBase::setupSequence(const std::vector<Real> & times)
{
  // In case of half transient, transient's end time needs to be reset to
  // be able to imprint TimeSequenceStepperBase's end time
  if (_app.halfTransient())
    _executioner.endTime() *= 2.0;

  // only set up _time_sequence if the app is _not_ restarting or recovering
  if (!_app.isRecovering() && ! _app.isRestarting())
  {
    // sync _executioner.startTime and endTime with _time_sequence
    Real start_time = _executioner.getStartTime();
    Real end_time = _executioner.endTime();

    // make sure time sequence is in ascending order
    for (unsigned int j = 0; j < times.size() - 1; ++j)
      if (times[j + 1] <= times[j])
        mooseError("time_sequence must be in ascending order.");

    _time_sequence.push_back(start_time);
    for (unsigned int j = 0; j < times.size(); ++j)
    {
      if (times[j] > start_time && times[j] < end_time)
        _time_sequence.push_back(times[j]);
    }
    _time_sequence.push_back(end_time);
  }

  if (_app.halfTransient())
  {
    unsigned int half = (_time_sequence.size() - 1) / 2;
    _executioner.endTime() = _time_sequence[half];
  }
}

void
TimeSequenceStepperBase::step()
{
  TimeStepper::step();
  if (converged())
    _current_step++;
}

Real
TimeSequenceStepperBase::computeInitialDT()
{
  return computeDT();
}

Real
TimeSequenceStepperBase::computeDT()
{
  return _time_sequence[_current_step + 1] - _time_sequence[_current_step];
}

Real
TimeSequenceStepperBase::computeFailedDT()
{
  if (computeDT() <= _dt_min)
    mooseError("Solve failed and timestep already at or below dtmin, cannot continue!");

  // cut the time step in a half if possible
  Real dt = 0.5 * computeDT();
  if (dt < _dt_min)
    dt = _dt_min;
  _time_sequence.insert(_time_sequence.begin() + _current_step + 1, _time_sequence[_current_step] + dt);
  return computeDT();
}
