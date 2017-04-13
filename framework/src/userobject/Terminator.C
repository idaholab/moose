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
#include "Terminator.h"
#include "MooseApp.h"
#include "Executioner.h"

template <>
InputParameters
validParams<Terminator>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<std::string>(
      "expression",
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
