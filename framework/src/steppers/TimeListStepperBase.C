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

#include "TimeListStepperBase.h"
#include "FEProblem.h"
#include "Transient.h"

template<>
InputParameters validParams<TimeListStepperBase>()
{
  InputParameters params = validParams<Stepper>();

  params.addParam<StepperName>("incoming_stepper", "The name of the Stepper to get the current dt from.  If this is provided then the behavior will be to allow dt to change, but the time will be forced to hit the times in the time list.  If not provided, then solves will occur only at exactly the times in the time list.");

  return params;
}

TimeListStepperBase::TimeListStepperBase(const InputParameters & parameters) :
    Stepper(parameters),
    _incoming_stepper_dt(getStepperDT("incoming_stepper"))
{
}

void
TimeListStepperBase::setupList(const std::vector<Real> & times)
{
  Real start_time = _executioner.getStartTime();
  Real end_time = _executioner.endTime();
  if (_app.halfTransient())
    end_time *= 2.0;

  _time_list.push_back(start_time);
  for (unsigned int j = 0; j < times.size(); ++j)
  {
    if (times[j] > start_time && times[j] < end_time)
      _time_list.push_back(times[j]);
  }
  _time_list.push_back(end_time);

  if (_app.halfTransient())
  {
    mooseAssert(_time_list.size() >= 1, "Not enough time steps for a half transient!");

    unsigned int half = (_time_list.size() - 1.) / 2.;
    _executioner.endTime() = _time_list[half];
  }

  // We'll use a FixedTimesStepper to hit the correct times...
  auto output_name = outputName();
  setOutputName(uName("start"));

  auto params = _factory.getValidParams("FixedTimesStepper");
  params.set<StepperName>("incoming_stepper") = outputName();
  params.set<std::vector<Real> >("times") = _time_list;
  params.set<StepperName>("_output_name") = output_name;
  _fe_problem_base.addStepper("FixedTimesStepper", uName("fixed"), params);
}

Real
TimeListStepperBase::computeInitialDT()
{
  return _incoming_stepper_dt;
}

Real
TimeListStepperBase::computeDT()
{
  // Keep in mind that if this wasn't coupled in, this will be max()
  // That works out perfectly with FixedTimesStepper (which will receive this value)
  // because it will cause it to take only exactly the steps in the time_list (which is the right behavior)
  return _incoming_stepper_dt;
}

Real
TimeListStepperBase::computeFailedDT()
{
  if (isParamValid("incoming_stepper"))
    return _incoming_stepper_dt;
  else
    return 0.5 * _dt[0];
}
