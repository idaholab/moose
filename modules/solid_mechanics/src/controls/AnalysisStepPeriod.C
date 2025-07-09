//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AnalysisStepPeriod.h"
#include "AnalysisStepUserObject.h"
#include "Function.h"
#include "Transient.h"
#include "MooseUtils.h"

registerMooseObject("SolidMechanicsApp", AnalysisStepPeriod);

InputParameters
AnalysisStepPeriod::validParams()
{
  InputParameters params = TimePeriodBase::validParams();
  params.addClassDescription(
      "Control the enabled/disabled state of objects with user-provided analysis steps.");
  params.addParam<bool>(
      "set_sync_times", true, "Set the start and end time as execute sync times.");
  params.addParam<UserObjectName>(
      "analysis_step_user_object", "The AnalysisStepUserObject that provides times from analysis steps.");
  params.addRequiredParam<unsigned int>("step_number",
                                        "Step number on which this control object applies.");
  return params;
}

AnalysisStepPeriod::AnalysisStepPeriod(const InputParameters & parameters) : TimePeriodBase(parameters) {}

void
AnalysisStepPeriod::initialSetup()
{
  // Let's automatically detect uos and identify the one we are interested in.
  // If there is more than one, we assume something is off and error out.
  if (!isParamSetByUser("step_user_object"))
    getAnalysisStepUserObject(_fe_problem, _step_user_object, name());
  else
    _step_user_object = &getUserObject<AnalysisStepUserObject>("analysis_step_user_object");

  _start_time.resize(1);
  _end_time.resize(1);

  // Set start time
  _start_time[0] = _step_user_object->getStartTime(getParam<unsigned int>("step_number"));

  // Set end time
  _end_time[0] = _step_user_object->getEndTime(getParam<unsigned int>("step_number"));

  // Call base method to populate control times.
  TimePeriodBase::setupTimes();

  if (getParam<bool>("set_sync_times"))
  {
    std::set<Real> & sync_times = _app.getOutputWarehouse().getSyncTimes();
    sync_times.insert(_start_time.begin(), _start_time.end());
    sync_times.insert(_end_time.begin(), _end_time.end());
  }
}

bool
AnalysisStepPeriod::conditionMet(const unsigned int & /*i*/)
{
  return MooseUtils::absoluteFuzzyGreaterEqual(_t, _start_time[0]) &&
         MooseUtils::absoluteFuzzyLessThan(_t, _end_time[0]);
}
