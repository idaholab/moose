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
    _time_sequence(declareRestartableData<std::vector<Real> >("time_sequence"))
{
}

void
TimeSequenceStepperBase::setupSequence(const std::vector<Real> & times)
{
  Real start_time = _executioner.getStartTime();
  Real end_time = _executioner.endTime();
  if (_app.halfTransient())
    end_time *= 2.0;

  // only set up _time_sequence if the app is _not_ recovering
  if (!_app.isRecovering())
  {
    // also we need to do something different when restarting
    if (!_app.isRestarting())
    {
      _time_sequence.push_back(start_time);
      for (unsigned int j = 0; j < times.size(); ++j)
      {
        if (times[j] > start_time && times[j] < end_time)
          _time_sequence.push_back(times[j]);
      }
      _time_sequence.push_back(end_time);
    }
    else
    {
      // in case of restart it should be allowed to modify _time_sequence if it follows the following rule:
      // all times up to current_step are identical
      // 1. start time cannot be modified
      // 2. the entries in _time_sequence and times must be equal up to entry with index current_step

      if (!MooseUtils::absoluteFuzzyEqual(_executioner.getStartTime(), _time_sequence[0]))
        mooseError("Timesequencestepper does not allow the start time to be modified.");

      // save the restarted time_sequence
      std::vector<Real> saved_time_sequence = _time_sequence;
      _time_sequence.clear();

      // step 1: fill in the entries up to current_step
      int current_step = _executioner.timeStep();
      for (unsigned int j = 0; j <= current_step; ++j)
      {
        if (!MooseUtils::absoluteFuzzyEqual(times[j], saved_time_sequence[j]))
          mooseError("The timesequence provided in the restart file must be identical to "
                     "the one in the old file up to entry number " << current_step + 1 << " = "
                     << saved_time_sequence[current_step]);
        _time_sequence.push_back(saved_time_sequence[j]);
      }

      // step 2: fill in the entries up after current_step
      for (unsigned int j = current_step + 1; j < times.size(); ++j)
      {
        if (times[j] < end_time)
          _time_sequence.push_back(times[j]);
      }
      _time_sequence.push_back(end_time);
    }
  }

  if (_app.halfTransient())
  {
    unsigned int half = (_time_sequence.size() - 1) / 2;
    _executioner.endTime() = _time_sequence[half];
  }
}

Stepper*
TimeSequenceStepperBase::buildStepper()
{
  return new IfConvergedStepper(new FixedPointStepper(_time_sequence, 0), new GrowShrinkStepper(0.5, 1.0));
}
