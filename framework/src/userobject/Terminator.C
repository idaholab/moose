//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Terminator.h"
#include "MooseApp.h"
#include "Executioner.h"

template <>
InputParameters
validParams<Terminator>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredCustomTypeParam<std::string>(
      "expression",
      "FunctionExpression",
      "FParser expression to process Postprocessor values into a boolean value. "
      "Termination of the simulation occurs when this returns true.");
  return params;
}

Terminator::Terminator(const InputParameters & parameters)
  : GeneralUserObject(parameters),
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

  _params = new Real[_pp_num];
}

Terminator::~Terminator() { delete[] _params; }

void
Terminator::execute()
{
  // copy current Postprocessor values into the FParser parameter buffer
  for (unsigned int i = 0; i < _pp_num; ++i)
    _params[i] = *(_pp_values[i]);

  // request termination of the run in case the expression evaluates to true
  if (_fp.Eval(_params) != 0)
    _fe_problem.terminateSolve();
}
