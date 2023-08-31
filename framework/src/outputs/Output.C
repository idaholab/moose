//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Standard includes
#include <cmath>
#include <limits>

// MOOSE includes
#include "Output.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MooseApp.h"
#include "Postprocessor.h"
#include "Restartable.h"
#include "FileMesh.h"
#include "MooseUtils.h"
#include "MooseApp.h"
#include "Console.h"
#include "Function.h"
#include "PiecewiseLinear.h"
#include "Times.h"

#include "libmesh/equation_systems.h"

InputParameters
Output::validParams()
{
  // Get the parameters from the parent object
  InputParameters params = MooseObject::validParams();
  params += SetupInterface::validParams();

  // Displaced Mesh options
  params.addParam<bool>(
      "use_displaced", false, "Enable/disable the use of the displaced mesh for outputting");

  // Output intervals and timing
  params.addParam<unsigned int>(
      "interval", 1, "The interval at which time steps are output to the solution file");
  params.addParam<Real>(
      "minimum_time_interval", 0.0, "The minimum simulation time between output steps");
  params.addParam<std::vector<Real>>("sync_times",
                                     "Times at which the output and solution is forced to occur");
  params.addParam<TimesName>(
      "sync_times_object",
      "Times object providing the times at which the output and solution is forced to occur");
  params.addParam<bool>("sync_only", false, "Only export results at sync times");
  params.addParam<Real>("start_time", "Time at which this output object begins to operate");
  params.addParam<Real>("end_time", "Time at which this output object stop operating");
  params.addParam<int>("start_step", "Time step at which this output object begins to operate");
  params.addParam<int>("end_step", "Time step at which this output object stop operating");
  params.addParam<Real>(
      "time_tolerance", 1e-14, "Time tolerance utilized checking start and end times");
  params.addDeprecatedParam<FunctionName>(
      "output_limiting_function",
      "Piecewise base function that sets sync_times",
      "Replaced by using the Times system with the sync_times_objects parameter");

  // Update the 'execute_on' input parameter for output
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum = Output::getDefaultExecFlagEnum();
  exec_enum = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  params.setDocString("execute_on", exec_enum.getDocString());

  // Add ability to append to the 'execute_on' list
  params.addParam<ExecFlagEnum>("additional_execute_on", exec_enum, exec_enum.getDocString());
  params.set<ExecFlagEnum>("additional_execute_on").clear();
  params.addParamNamesToGroup("execute_on additional_execute_on", "Execution scheduling");

  // 'Timing' group
  params.addParamNamesToGroup(
      "time_tolerance interval sync_times sync_times_object sync_only start_time end_time "
      "start_step end_step minimum_time_interval",
      "Timing and frequency of output");

  // Add a private parameter for indicating if it was created with short-cut syntax
  params.addPrivateParam<bool>("_built_by_moose", false);

  // Register this class as base class
  params.declareControllable("enable");
  params.registerBase("Output");

  return params;
}

ExecFlagEnum
Output::getDefaultExecFlagEnum()
{
  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum.addAvailableFlags(EXEC_FAILED);
  return exec_enum;
}

Output::Output(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(this, "Output"),
    MeshChangedInterface(parameters),
    SetupInterface(this),
    FunctionInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    ReporterInterface(this),
    PerfGraphInterface(this),
    _problem_ptr(getParam<FEProblemBase *>("_fe_problem_base")),
    _transient(_problem_ptr->isTransient()),
    _use_displaced(getParam<bool>("use_displaced")),
    _es_ptr(nullptr),
    _mesh_ptr(nullptr),
    _execute_on(getParam<ExecFlagEnum>("execute_on")),
    _current_execute_flag(EXEC_NONE),
    _time(_problem_ptr->time()),
    _time_old(_problem_ptr->timeOld()),
    _t_step(_problem_ptr->timeStep()),
    _dt(_problem_ptr->dt()),
    _dt_old(_problem_ptr->dtOld()),
    _num(0),
    _interval(getParam<unsigned int>("interval")),
    _minimum_time_interval(getParam<Real>("minimum_time_interval")),
    _sync_times(std::set<Real>(getParam<std::vector<Real>>("sync_times").begin(),
                               getParam<std::vector<Real>>("sync_times").end())),
    _sync_times_object(isParamValid("sync_times_object")
                           ? static_cast<Times *>(&_problem_ptr->getUserObject<Times>(
                                 getParam<TimesName>("sync_times_object")))
                           : nullptr),
    _start_time(isParamValid("start_time") ? getParam<Real>("start_time")
                                           : std::numeric_limits<Real>::lowest()),
    _end_time(isParamValid("end_time") ? getParam<Real>("end_time")
                                       : std::numeric_limits<Real>::max()),
    _start_step(isParamValid("start_step") ? getParam<int>("start_step")
                                           : std::numeric_limits<int>::lowest()),
    _end_step(isParamValid("end_step") ? getParam<int>("end_step")
                                       : std::numeric_limits<int>::max()),
    _t_tol(getParam<Real>("time_tolerance")),
    _sync_only(getParam<bool>("sync_only")),
    _allow_output(true),
    _is_advanced(false),
    _advanced_execute_on(_execute_on, parameters),
    _last_output_time(
        declareRestartableData<Real>("last_output_time", std::numeric_limits<Real>::lowest()))
{
  if (_use_displaced)
  {
    std::shared_ptr<DisplacedProblem> dp = _problem_ptr->getDisplacedProblem();
    if (dp != nullptr)
    {
      _es_ptr = &dp->es();
      _mesh_ptr = &dp->mesh();
    }
    else
    {
      mooseWarning(
          name(),
          ": Parameter 'use_displaced' ignored, there is no displaced problem in your simulation.");
      _es_ptr = &_problem_ptr->es();
      _mesh_ptr = &_problem_ptr->mesh();
    }
  }
  else
  {
    _es_ptr = &_problem_ptr->es();
    _mesh_ptr = &_problem_ptr->mesh();
  }

  // Apply the additional output flags
  if (isParamValid("additional_execute_on"))
  {
    const ExecFlagEnum & add = getParam<ExecFlagEnum>("additional_execute_on");
    for (auto & me : add)
      _execute_on.push_back(me);
  }

  if (isParamValid("output_limiting_function"))
  {
    const Function & olf = getFunction("output_limiting_function");
    const PiecewiseBase * pwb_olf = dynamic_cast<const PiecewiseBase *>(&olf);
    if (pwb_olf == nullptr)
      mooseError("Function muse have a piecewise base!");

    for (auto i = 0; i < pwb_olf->functionSize(); i++)
      _sync_times.insert(pwb_olf->domain(i));
  }

  // Get sync times from Times object if using
  if (_sync_times_object)
  {
    if (isParamValid("output_limiting_function") || isParamSetByUser("sync_times"))
      paramError("sync_times_object",
                 "Only one method of specifying sync times is supported at a time");
    else
      // Sync times for the time steppers are taken from the output warehouse. The output warehouse
      // takes sync times from the output objects immediately after the object is constructed. Hence
      // we must ensure that we set the `_sync_times` in the constructor
      _sync_times = _sync_times_object->getUniqueTimes();
  }
}

void
Output::solveSetup()
{
}

void
Output::outputStep(const ExecFlagType & type)
{
  // Output is not allowed
  if (!_allow_output && type != EXEC_FORCED)
    return;

  // If recovering disable output of initial condition, it was already output
  if (type == EXEC_INITIAL && _app.isRecovering())
    return;

  // Return if the current output is not on the desired interval
  if (type != EXEC_FINAL && !onInterval())
    return;

  // store current simulation time
  _last_output_time = _time;

  // set current type
  _current_execute_flag = type;

  // Call the output method
  if (shouldOutput())
  {
    TIME_SECTION("outputStep", 2, "Outputting Step");
    output();
  }

  _current_execute_flag = EXEC_NONE;
}

bool
Output::shouldOutput()
{
  if (_execute_on.contains(_current_execute_flag) || _current_execute_flag == EXEC_FORCED)
    return true;
  return false;
}

bool
Output::onInterval()
{
  // The output flag to return
  bool output = false;

  // Return true if the current step on the current output interval and within the output time range
  // and within the output step range
  if (_time >= _start_time && _time <= _end_time && _t_step >= _start_step &&
      _t_step <= _end_step && (_t_step % _interval) == 0)
    output = true;

  // Return false if 'sync_only' is set to true
  if (_sync_only)
    output = false;

  if (_sync_times_object)
  {
    const auto & sync_times = _sync_times_object->getUniqueTimes();
    if (sync_times != _sync_times)
      mooseError("The provided sync times object has changing time values. Only static time "
                 "values are supported since time steppers take sync times from the output "
                 "warehouse which determines its sync times at output construction time.");
  }

  // If sync times are not skipped, return true if the current time is a sync_time
  if (_sync_times.find(_time) != _sync_times.end())
    output = true;

  // check if enough time has passed between outputs
  if (_time > _last_output_time && _last_output_time + _minimum_time_interval > _time + _t_tol)
    return false;

  // Return the output status
  return output;
}

Real
Output::time()
{
  if (_transient)
    return _time;
  else
    return _t_step;
}

Real
Output::timeOld()
{
  if (_transient)
    return _time_old;
  else
    return _t_step - 1;
}

Real
Output::dt()
{
  if (_transient)
    return _dt;
  else
    return 1;
}

Real
Output::dtOld()
{
  if (_transient)
    return _dt_old;
  else
    return 1;
}

int
Output::timeStep()
{
  return _t_step;
}

const MultiMooseEnum &
Output::executeOn() const
{
  return _execute_on;
}

bool
Output::isAdvanced()
{
  return _is_advanced;
}

const OutputOnWarehouse &
Output::advancedExecuteOn() const
{
  mooseError("The output object ", name(), " is not an AdvancedOutput, use isAdvanced() to check.");
  return _advanced_execute_on;
}
