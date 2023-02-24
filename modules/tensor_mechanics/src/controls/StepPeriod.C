//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "StepPeriod.h"
#include "StepUserObject.h"
#include "Function.h"
#include "Transient.h"
#include "MooseUtils.h"

registerMooseObject("TensorMechanicsApp", StepPeriod);

InputParameters
StepPeriod::validParams()
{
  InputParameters params = TimePeriodBase::validParams();
  params.addClassDescription(
      "Control the enabled/disabled state of objects with user-provided simulation steps.");
  params.addParam<bool>(
      "set_sync_times", true, "Set the start and end time as execute sync times.");
  params.addParam<UserObjectName>(
      "step_user_object", "The StepUserObject that provides times from simulation loading steps.");
  params.addRequiredParam<unsigned int>("step_number",
                                        "Step number on which this control object applies.");
  return params;
}

StepPeriod::StepPeriod(const InputParameters & parameters) : TimePeriodBase(parameters) {}

void
StepPeriod::initialSetup()
{
  // Let's automatically detect uos and identify the one we are interested in.
  // If there is more than one, we assume something is off and error out.
  if (!isParamSetByUser("step_user_object"))
  {
    std::vector<const UserObject *> uos;
    _fe_problem.theWarehouse().query().condition<AttribSystem>("UserObject").queryIntoUnsorted(uos);

    std::vector<const StepUserObject *> step_uos;
    for (const auto & uo : uos)
    {
      const StepUserObject * possible_step_uo = dynamic_cast<const StepUserObject *>(uo);
      if (possible_step_uo)
        step_uos.push_back(possible_step_uo);
    }

    if (step_uos.size() > 1)
      mooseError(
          "Your input file has multiple StepUserObjects. MOOSE currently only support one in ",
          name(),
          ". \n");
    else if (step_uos.size() == 1)
      mooseInfo("A StepUserObject, ",
                step_uos[0]->name(),
                ", has been identified and will be used to drive stepping behavior in ",
                name(),
                ".");

    _step_user_object = step_uos.size() == 1 ? step_uos[0] : nullptr;
  }
  else
    _step_user_object = &getUserObject<StepUserObject>("step_user_object");

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
StepPeriod::conditionMet(const unsigned int & /*i*/)
{
  return MooseUtils::absoluteFuzzyGreaterEqual(_t, _start_time[0]) &&
         MooseUtils::absoluteFuzzyLessThan(_t, _end_time[0]);
}
