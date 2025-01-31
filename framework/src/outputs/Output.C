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
  params.addRangeCheckedParam<unsigned int>(
      "time_step_interval",
      1,
      "time_step_interval > 0",
      "The interval (number of time steps) at which output occurs. "
      "Unless explicitly set, the default value of this parameter is set "
      "to infinity if the wall_time_interval is explicitly set.");
  params.addParam<unsigned int>("interval",
                                "The interval (number of time steps) at which output occurs");
  params.deprecateParam("interval", "time_step_interval", "02/01/2025");
  params.addParam<Real>(
      "min_simulation_time_interval", 0.0, "The minimum simulation time between output steps");
  params.addParam<Real>("minimum_time_interval",
                        "The minimum simulation time between output steps");
  params.deprecateParam("minimum_time_interval", "min_simulation_time_interval", "02/01/2025");
  params.addParam<Real>("simulation_time_interval",
                        std::numeric_limits<Real>::max(),
                        "The target simulation time interval (in seconds) at which to output");
  params.addRangeCheckedParam<Real>(
      "wall_time_interval",
      std::numeric_limits<Real>::max(),
      "wall_time_interval > 0",
      "The target wall time interval (in seconds) at which to output");
  params.addParam<std::vector<Real>>(
      "sync_times", {}, "Times at which the output and solution is forced to occur");
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
  params.set<ExecFlagEnum>("additional_execute_on").clearSetValues();
  params.addParamNamesToGroup("execute_on additional_execute_on", "Execution scheduling");

  // 'Timing' group
  params.addParamNamesToGroup("time_tolerance time_step_interval sync_times sync_times_object "
                              "sync_only start_time end_time "
                              "start_step end_step min_simulation_time_interval "
                              "simulation_time_interval wall_time_interval",
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
    _time_step_interval_set_by_addparam(parameters.isParamSetByAddParam("time_step_interval")),
    // If wall_time_interval is user-specified and time_step_interval is not,
    // override default value of time_step_interval so output does not occur
    // after every time step.
    _time_step_interval(
        (parameters.isParamSetByUser("wall_time_interval") && _time_step_interval_set_by_addparam)
            ? std::numeric_limits<unsigned int>::max()
            : getParam<unsigned int>("time_step_interval")),
    _min_simulation_time_interval(getParam<Real>("min_simulation_time_interval")),
    _simulation_time_interval(getParam<Real>("simulation_time_interval")),
    _wall_time_interval(getParam<Real>("wall_time_interval")),
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
    _last_output_simulation_time(declareRestartableData<Real>("last_output_simulation_time",
                                                              std::numeric_limits<Real>::lowest())),
    _last_output_wall_time(std::chrono::steady_clock::now())
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
      _execute_on.setAdditionalValue(me);
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

  // Return if the current output is not on the desired interval and there is
  // no signal to process
  const bool on_interval_or_exec_final = (onInterval() || (type == EXEC_FINAL));
  // Sync across processes and only output one time per signal received.
  comm().max(Moose::interrupt_signal_number);
  const bool signal_received = Moose::interrupt_signal_number;
  if (!(on_interval_or_exec_final || signal_received))
    return;

  // set current type
  _current_execute_flag = type;

  // Check whether we should output, then do it.
  if (shouldOutput())
  {
    // store current simulation time
    _last_output_simulation_time = _time;

    // store current wall time of output
    _last_output_wall_time = std::chrono::steady_clock::now();

    TIME_SECTION("outputStep", 2, "Outputting Step");
    output();
  }

  _current_execute_flag = EXEC_NONE;
}

bool
Output::shouldOutput()
{
  if (_execute_on.isValueSet(_current_execute_flag) || _current_execute_flag == EXEC_FORCED)
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
      _t_step <= _end_step && (_t_step % _time_step_interval) == 0)
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
  for (const auto _sync_time : _sync_times)
  {
    if (std::abs(_sync_time - _time) < _t_tol)
      output = true;
  }

  // check if enough simulation time has passed between outputs
  if (_time > _last_output_simulation_time &&
      _last_output_simulation_time + _min_simulation_time_interval > _time + _t_tol)
    output = false;

  // check if enough wall time has passed between outputs
  const auto now = std::chrono::steady_clock::now();
  // count below returns an interger type, so lets express on a millisecond
  // scale and convert to seconds for finer resolution
  _wall_time_since_last_output =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - _last_output_wall_time).count() /
      1000.0;
  // Take the maximum wall time since last output accross all processors
  _communicator.max(_wall_time_since_last_output);
  if (_wall_time_since_last_output >= _wall_time_interval)
    output = true;

  // Return the output status
  return output;
}

void
Output::setWallTimeIntervalFromCommandLineParam()
{
  if (_app.isParamValid("output_wall_time_interval"))
  {
    _wall_time_interval = _app.getParam<Real>("output_wall_time_interval");

    // If default value of _wall_time_interval was just overriden and user did not
    // explicitly specify _time_step_interval, override default value of
    // _time_step_interval so output does not occur after every time step
    if (_time_step_interval_set_by_addparam)
      _time_step_interval = std::numeric_limits<unsigned int>::max();
  }
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
