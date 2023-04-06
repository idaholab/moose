//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeStepperSystem.h"
#include "TimeSequenceStepperBase.h"
#include "FEProblem.h"
#include "SubProblem.h"
#include "Transient.h"
#include "InputParameters.h"

#include <regex>

TimeStepperSystem::TimeStepperSystem(MooseApp & app)
  : PerfGraphInterface(app.perfGraph(), "TimeStepperSystem"), ParallelObject(app), _app(app)
{
}

TimeStepperSystem::~TimeStepperSystem() {}

void
TimeStepperSystem::addTimeStepper(const std::string & type,
                                  const std::string & name,
                                  const InputParameters & params)
{
  mooseAssert(!_time_steppers.count(name), "Already exists");

  auto new_params = params;

  Transient * transient = dynamic_cast<Transient *>(_app.getExecutioner());
  SubProblem * subproblem = dynamic_cast<SubProblem *>(&_app.feProblem());

  // Set required parameters
  new_params.set<SubProblem *>("_subproblem") = subproblem;
  new_params.set<Transient *>("_executioner") = transient;

  if (name == "TimeStepper")
    mooseError("The user-defined time stepper name: '", name, "' is a reserved name");

  // set the subproblem, set
  // Need to add this to the param map so that createTimeStepper can use it
  _time_stepper_params.emplace(std::piecewise_construct,
                               std::forward_as_tuple(name),
                               std::forward_as_tuple(type, new_params));
}

void
TimeStepperSystem::createAddedTimeSteppers()
{
  // No TimeSteppers were added via addTimeStepper()
  if (_time_stepper_params.empty())
    return;

  // Store the name and dt of the time stepper except the final one into a map
  for (const auto & [name, type_params_pair] : _time_stepper_params)
  {
    if (name != _final_time_stepper_name)
    {
      _time_steppers.emplace(name, createTimeStepper(name, type_params_pair));
    }
  }

  // Create and store the final time stepper here because composition time stepper depends on other
  // time steppers
  _time_steppers.emplace(
      _final_time_stepper_name,
      createTimeStepper(_final_time_stepper_name,
                        _time_stepper_params.find(_final_time_stepper_name)->second));
}

std::shared_ptr<TimeStepper>
TimeStepperSystem::createTimeStepper(
    const std::string & stepper_name,
    const std::pair<std::string, InputParameters> & type_params_pair)
{
  libmesh_parallel_only(comm());

  const auto & [type, params] = type_params_pair;
  mooseAssert(comm().verify(type + stepper_name), "Inconsistent construction order");
  mooseAssert(!_time_steppers.count(stepper_name), "Already created");

  std::regex time_sequence(".*SequenceStepper");

  if (!std::regex_match(type, time_sequence))
  {
    std::shared_ptr<TimeStepper> ts =
        _app.getFactory().create<TimeStepper>(type, stepper_name, params);
    mooseAssert(ts, "Not successfully created");
    return ts;
  }

  std::shared_ptr<TimeSequenceStepperBase> ts =
      _app.getFactory().create<TimeSequenceStepperBase>(type, stepper_name, params);
  mooseAssert(ts, "Not successfully created");

  return ts;
}

std::shared_ptr<TimeStepper>
TimeStepperSystem::getFinalTimeStepper()
{
  auto final_stepper = getTimeStepper(_final_time_stepper_name);
  mooseAssert(final_stepper, "Not found");

  return final_stepper;
}

std::shared_ptr<TimeStepper>
TimeStepperSystem::getTimeStepper(const std::string & stepper_name)
{
  const auto find_stepper = _time_steppers.find(stepper_name);
  mooseAssert(find_stepper != _time_steppers.end(), "Not added");

  return find_stepper->second;
}

void
TimeStepperSystem::setFinalTimeStepperName()
{
  auto final_stepper = _app.getExecutioner()->getParam<std::string>("final_time_stepper");
  for (const auto & [name, type_params_pair] : _time_stepper_params)
  {
    const auto & type = type_params_pair.first;

    if (type == "CompositionDT")
      _final_time_stepper_name = name;
    else if (!final_stepper.empty())
      _final_time_stepper_name = final_stepper;
    else
      mooseError(
          "Multiple TimeSteppers are used, please either select one as final TimeStepper using "
          "'final_time_stepper' option or use 'CompositionDT' to create a composed DT.");
  }
}
