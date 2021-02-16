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

defineLegacyParams(Terminator);

InputParameters
Terminator::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Requests termination of the current solve based on the values of "
                             "Postprocessor value(s) via a logical expression.");
  params.addRequiredCustomTypeParam<std::string>(
      "expression",
      "FunctionExpression",
      "FParser expression to process Postprocessor values into a boolean value. "
      "Termination of the simulation occurs when this returns true.");
  MooseEnum failModeOption("HARD SOFT", "HARD");
  params.addParam<MooseEnum>(
      "fail_mode",
      failModeOption,
      "Abort entire simulation (HARD) or just the current time step (SOFT).");
  params.addParam<std::string>(
      "message", "An optional message to be output when the termination condition is triggered");

  MooseEnum errorLevel("INFO WARNING ERROR");
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
    _error_level(isParamValid("error_level")
                     ? getParam<MooseEnum>("error_level").getEnum<ErrorLevel>()
                     : ErrorLevel::NONE),
    _pp_names(),
    _pp_values(),
    _expression(getParam<std::string>("expression")),
    _fp()
{
  // sanity check the parameters
  if (_error_level == ErrorLevel::ERROR && _fail_mode == FailMode::SOFT)
    paramError("error_level", "Setting the error level to ERROR always causes a hard failure.");
  if (_error_level != ErrorLevel::NONE && !isParamValid("message"))
    paramError("error_level",
               "If this parameter is specified a `message` must be supplied as well.");

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
Terminator::handleMessage()
{
  if (!isParamValid("message"))
    return;

  auto message = getParam<std::string>("message");
  switch (_error_level)
  {
    case ErrorLevel::INFO:
      mooseInfo(message);
      break;

    case ErrorLevel::WARNING:
      mooseWarning(message);
      break;

    case ErrorLevel::ERROR:
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
    if (_fail_mode == FailMode::HARD)
    {
      handleMessage();
      _fe_problem.terminateSolve();
    }
    else
    {
      _console << name() << " is marking the current solve step as failed.\n";
      handleMessage();
      getMooseApp().getExecutioner()->picardSolve().failStep();
    }
  }
}

#endif
