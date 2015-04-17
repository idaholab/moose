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

// Standard includes
#include <math.h>

// MOOSE includes
#include "Output.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MooseApp.h"
#include "Postprocessor.h"
#include "Restartable.h"
#include "FileMesh.h"
#include "CoupledExecutioner.h"
#include "MooseUtils.h"

template<>
InputParameters validParams<Output>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<MooseObject>();

  // Displaced Mesh options
  params.addParam<bool>("use_displaced", false, "Enable/disable the use of the displaced mesh for outputting");

  // Output intervals and timing
  params.addParam<unsigned int>("interval", 1, "The interval at which time steps are output to the solution file");
  params.addParam<std::vector<Real> >("sync_times", "Times at which the output and solution is forced to occur");
  params.addParam<bool>("sync_only", false, "Only export results at sync times");
  params.addParam<Real>("start_time", "Time at which this output object begins to operate");
  params.addParam<Real>("end_time", "Time at which this output object stop operating");
  params.addParam<Real>("time_tolerance", 1e-14, "Time tolerance utilized checking start and end times");

  // Add the 'output_on' input parameter for users to set
  params.addParam<MultiMooseEnum>("output_on", Output::getExecuteOptions("timestep_end"), "Set to (none|initial|linear|nonlinear|timestep_end|timestep_begin|final|failed|custom) to execute only at that moment");

  // Add ability to append to the 'output_on' list
  params.addParam<MultiMooseEnum>("additional_output_on", Output::getExecuteOptions(), "This list of output flags is added to the existing flags (initial|linear|nonlinear|timestep_end|timestep_begin|final|failed|custom) to execute only at that moment");

  // 'Timing' group
  params.addParamNamesToGroup("time_tolerance interval output_initial output_final sync_times sync_only start_time end_time ", "Timing");

  // Output toggles
  params.addParam<bool>("output_initial", false, "Request that the initial condition is output to the solution file");
  params.addParam<bool>("output_timestep_end", true, "Request that data be output at the end of the timestep");
  params.addParam<bool>("output_final", false, "Force the final time step to be output, regardless of output interval");
  params.addParam<bool>("output_failed", false, "When true all time attempted time steps are output");

  // **** DEPRECATED PARAMETERS ****
  params.addDeprecatedParam<bool>("output_intermediate", true, "Request that all intermediate steps (not initial or final) are output", "Replace this output with 'output_timestep_end'");

  // Add a private parameter for indicating if it was created with short-cut syntax
  params.addPrivateParam<bool>("_built_by_moose", false);

  // Register this class as base class
  params.registerBase("Output");
  return params;
}

MultiMooseEnum
Output::getExecuteOptions(std::string default_type)
{
  // Build the string of options
  std::string options = "none=0x00 initial=0x01 linear=0x02 nonlinear=0x04 timestep_end=0x08 timestep_begin=0x10 final=0x20 failed=0x80";

  // The numbers associated must be in sync with the ExecFlagType in Moose.h
  return MultiMooseEnum(options, default_type);
}

Output::Output(const std::string & name, InputParameters & parameters) :
    MooseObject(name, parameters),
    Restartable(parameters, "Output"),
    MeshChangedInterface(parameters),
    SetupInterface(parameters),
    _problem_ptr(getParam<FEProblem *>("_fe_problem")),
    _transient(_problem_ptr->isTransient()),
    _use_displaced(getParam<bool>("use_displaced")),
    _es_ptr(_use_displaced ? &_problem_ptr->getDisplacedProblem()->es() : &_problem_ptr->es()),
    _output_on(getParam<MultiMooseEnum>("output_on")),
    _time(_problem_ptr->time()),
    _time_old(_problem_ptr->timeOld()),
    _t_step(_problem_ptr->timeStep()),
    _dt(_problem_ptr->dt()),
    _dt_old(_problem_ptr->dtOld()),
    _num(0),
    _interval(getParam<unsigned int>("interval")),
    _sync_times(std::set<Real>(getParam<std::vector<Real> >("sync_times").begin(), getParam<std::vector<Real> >("sync_times").end())),
    _start_time(isParamValid("start_time") ? getParam<Real>("start_time") : -std::numeric_limits<Real>::max()),
    _end_time(isParamValid("end_time") ? getParam<Real>("end_time") : std::numeric_limits<Real>::max()),
    _t_tol(getParam<Real>("time_tolerance")),
    _sync_only(getParam<bool>("sync_only")),
    _initialized(false),
    _allow_output(true),
    _is_advanced(false),
    _advanced_output_on(_output_on, parameters)
{

  // Handle the short-cut output flags
  applyOutputOnShortCutFlags(_output_on);

  // Apply the additional output flags
  MultiMooseEnum add = getParam<MultiMooseEnum>("additional_output_on");
  for (MooseEnumIterator it = add.begin(); it != add.end(); ++it)
    _output_on.push_back(*it);

  // **** DEPRECATED PARAMETER SUPPORT ****
  if (!getParam<bool>("output_intermediate"))
    _output_on.erase("timestep_end");
}

void
Output::initialSetup()
{
  _initialized = true;
}

void
Output::solveSetup()
{
}

bool
Output::shouldOutput(const ExecFlagType & type)
{
  if (_output_on.contains(type) || type == EXEC_FORCED)
    return true;
  return false;
}

bool
Output::onInterval()
{
  // The output flag to return
  bool output = false;

  // Return true if the current step on the current output interval and within the output time range
  if (_time >= _start_time && _time <= _end_time && (_t_step % _interval) == 0 )
    output = true;

  // Return false if 'sync_only' is set to true
  if (_sync_only)
    output = false;

  // If sync times are not skipped, return true if the current time is a sync_time
  if (_sync_times.find(_time) != _sync_times.end())
    output = true;

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
Output::outputOn() const
{
  return _output_on;
}

bool
Output::isAdvanced()
{
  return _is_advanced;
}

const OutputOnWarehouse &
Output::advancedOutputOn() const
{
  mooseError("The output object " << name() << " is not an AdvancedOutput, use isAdvanced() to check.");
  return _advanced_output_on;
}

void
Output::
applyOutputOnShortCutFlags(MultiMooseEnum & input)
{
  /*
   * output_initial/final/failed are false by default
   * If they are enabled, then update the output_on
   * to include this execution time.
   */
  if (getParam<bool>("output_initial"))
    input.push_back("initial");

  if (getParam<bool>("output_final"))
    input.push_back("final");

  if (getParam<bool>("output_failed"))
    input.push_back("failed");

  /*
   * output_timestep_end is true by default
   * If it is disabled, then update the output_on
   * by removing it from the execution time.
   */
  if (!getParam<bool>("output_timestep_end"))
    input.erase("timestep_end");
}
