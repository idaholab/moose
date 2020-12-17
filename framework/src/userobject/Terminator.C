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
  params.addParam<MooseEnum>("fail_mode",
                             failModeOption,
                             "Abort entire simulation (HARD) or just the current timestep (SOFT).");
  return params;
}

Terminator::Terminator(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _fail_mode(getParam<MooseEnum>("fail_mode").getEnum<FailMode>()),
    _pp_names(),
    _pp_values(),
    _expression(getParam<std::string>("expression")),
    _fp()
{
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
Terminator::execute()
{
  // copy current Postprocessor values into the FParser parameter buffer
  for (unsigned int i = 0; i < _pp_num; ++i)
    _params[i] = *(_pp_values[i]);

  // request termination of the run or timestep in case the expression evaluates to true
  if (_fp.Eval(_params.data()) != 0)
  {
    if (_fail_mode == FailMode::HARD)
      _fe_problem.terminateSolve();
    else
    {
      _console << name() << " is marking the current solve step as failed.\n";
      getMooseApp().getExecutioner()->picardSolve().failStep();
    }
  }
}

#endif
