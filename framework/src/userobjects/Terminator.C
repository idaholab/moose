//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_FPARSER

#include "Terminator.h"
#include "MooseApp.h"
#include "MooseEnum.h"
#include "Executioner.h"

registerMooseObject("MooseApp", Terminator);

InputParameters
Terminator::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Requests termination of the current solve based on the evaluation of"
                             " a parsed logical expression of the Postprocessor value(s).");
  params.addRequiredCustomTypeParam<std::string>(
      "expression",
      "FunctionExpression",
      "FParser expression to process Postprocessor values into a boolean value. "
      "Termination of the simulation occurs when this returns true.");
  MooseEnum failModeOption("HARD SOFT NONE", "HARD");
  params.addParam<MooseEnum>(
      "fail_mode",
      failModeOption,
      "Abort entire simulation (HARD), just the current time step (SOFT), or not at all (NONE).");
  params.addParam<std::string>("message",
                               "An optional message to be output instead of the default message "
                               "when the termination condition is triggered");

  MooseEnum errorLevel("INFO WARNING ERROR NONE", "INFO");
  errorLevel.addDocumentation("INFO", "Output an information message once.");
  errorLevel.addDocumentation("WARNING", "Output a warning message once.");
  errorLevel.addDocumentation("ERROR",
                              "Throw a MOOSE error, resulting in the termination of the run.");
  errorLevel.addDocumentation("NONE", "No message will be printed.");
  params.addParam<MooseEnum>(
      "error_level",
      errorLevel,
      "The error level for the message. A level of ERROR will always lead to a hard "
      "termination of the entire simulation.");
  return params;
}

Terminator::Terminator(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _fail_mode(getParam<MooseEnum>("fail_mode").getEnum<FailMode>()),
    _msg_type(getParam<MooseEnum>("error_level").getEnum<MessageType>()),
    _pp_names(),
    _pp_values(),
    _expression(getParam<std::string>("expression")),
    _fp()
{
  // sanity check the parameters
  if (_msg_type == MessageType::ERROR && _fail_mode != FailMode::HARD)
    paramError("error_level",
               "Setting the error level to ERROR always causes a hard failure, which is "
               "incompatible with `fail_mode=SOFT or NONE`.");
  if (_msg_type == MessageType::NONE && isParamValid("message"))
    paramError("error_level",
               "Cannot specify `error_level=NONE` together with the `message` parameter.");
  if (_msg_type == MessageType::NONE && _fail_mode == FailMode::NONE)
    paramWarning("error_level",
                 "With the current error level and fail mode settings, the terminator will not "
                 "error or output.");

  // build the expression object
  if (_fp.ParseAndDeduceVariables(_expression, _pp_names) >= 0)
    mooseError(std::string("Invalid function\n" + _expression + "\nin Terminator.\n") +
               _fp.ErrorMsg());

  _pp_num = _pp_names.size();
  _pp_values.resize(_pp_num);

  // get all necessary postprocessors
  for (unsigned int i = 0; i < _pp_num; ++i)
    _pp_values[i] = &getPostprocessorValueByName(_pp_names[i]);

  _params.resize(_pp_num);
}

void
Terminator::initialSetup()
{
  // Check execution schedule of the postprocessors
  if (_fail_mode == FailMode::SOFT || _fail_mode == FailMode::NONE)
    for (const auto i : make_range(_pp_num))
    // Make sure the postprocessor is executed at least as often
    {
      const auto & pp_exec = _fe_problem.getUserObjectBase(_pp_names[i], _tid).getExecuteOnEnum();
      for (const auto & flag : getExecuteOnEnum())
        if (!pp_exec.isValueSet(flag) && flag != EXEC_FINAL)
          paramWarning("expression",
                       "Postprocessor '" + _pp_names[i] + "' is not executed on " + flag.name() +
                           ", which it really should be to serve in the criterion "
                           "expression for throwing.");
    }
}

void
Terminator::handleMessage()
{
  std::string message;
  if (!isParamValid("message"))
  {
    message = "Terminator '" + name() + "' is ";
    if (_fail_mode == FailMode::HARD)
      message += "causing the execution to terminate.\n";
    else if (_fail_mode == FailMode::SOFT)
      message += "causing a time step cutback by marking the current step as failed.\n";
    else
      message += "outputting a message due to the criterion being met";
  }
  else
    message = getParam<std::string>("message");

  switch (_msg_type)
  {
    case MessageType::INFO:
      mooseInfoRepeated(message);
      break;

    case MessageType::WARNING:
      mooseWarning(message);
      break;

    case MessageType::ERROR:
      mooseError(message);
      break;

    default:
      break;
  }
}

void
Terminator::execute()
{
  // copy current Postprocessor values into the FParser parameter buffer
  for (unsigned int i = 0; i < _pp_num; ++i)
    _params[i] = *(_pp_values[i]);

  // request termination of the run or timestep in case the expression evaluates to true
  if (_fp.Eval(_params.data()) != 0)
  {
    handleMessage();
    if (_fail_mode == FailMode::HARD)
      _fe_problem.terminateSolve();
    else if (_fail_mode == FailMode::SOFT)
    {
      // Within a nonlinear solve, trigger a solve fail
      if (_fe_problem.getCurrentExecuteOnFlag() == EXEC_LINEAR ||
          _fe_problem.getCurrentExecuteOnFlag() == EXEC_NONLINEAR)
        _fe_problem.setFailNextNonlinearConvergenceCheck();
      // Outside of a solve, trigger a time step fail
      else
        getMooseApp().getExecutioner()->fixedPointSolve().failStep();
    }
  }
}

#endif
