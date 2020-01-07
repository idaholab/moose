//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "IntermittentFailureUO.h"

#include <thread>
#include <chrono>
#include <cstdlib>

registerMooseObject("MooseTestApp", IntermittentFailureUO);

InputParameters
IntermittentFailureUO::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<unsigned int>("timestep_to_fail", 1, "The timestep number to error out");

  MooseEnum failure_type("RUN_SLOW", "RUN_SLOW");
  params.addParam<MooseEnum>("failure_type", failure_type, "The type of failure to produce");

  params.addParam<FileName>(
      "state_file",
      "_has_failed_once",
      "A filename that will be \"touched\" indicating whether this is the first time this "
      "simulation has run or not, triggering this Postprocessor to stall.");

  return params;
}

IntermittentFailureUO::IntermittentFailureUO(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _state_file(getParam<FileName>("state_file")),
    _timestep_to_fail(getParam<unsigned int>("timestep_to_fail")),
    _failure_type(getParam<MooseEnum>("failure_type").getEnum<FailureType>()),
    _will_fail_this_run(true)
{
}

void
IntermittentFailureUO::initialSetup()
{
  char * intermittent_failures = std::getenv("MOOSE_ENABLE_INTERMITTENT_FAILURES");

  if (!intermittent_failures || MooseUtils::checkFileReadable(_state_file, false, false))
    _will_fail_this_run = false;
  else
  {
    std::ofstream out(_state_file.c_str(), std::ofstream::out);
    out.close();
    mooseInfo("IntermittentFailureUO present and will trigger");
  }
}

void
IntermittentFailureUO::execute()
{
  // Only fail the first time (simulation)!
  if (_will_fail_this_run && static_cast<unsigned int>(_t_step) >= _timestep_to_fail)
  {
    switch (_failure_type)
    {
      case FailureType::RUN_SLOW:
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        break;
      default:
        mooseError("Unknown failure type:");
    }
  }
}
