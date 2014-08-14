#include "Terminator.h"
#include "MooseApp.h"
#include "Executioner.h"

template<>
InputParameters validParams<Terminator>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<std::vector<std::string> >("postprocessors", "List of Postprocessors used in the expression");
  params.addRequiredParam<std::string>("expression", "FParser expression to process the Postprocessor values into a boolean value. Termination of the simulation occurs when this returns true.");
  return params;
}

Terminator::Terminator(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters),
    _pp_names(getParam<std::vector<std::string> >("postprocessors")),
    _pp_num(_pp_names.size()),
    _pp_values(_pp_num),
    _expression(getParam<std::string>("expression")),
    _fp()
{
  // build 'variables' argument for fparser
  std::string variables = _pp_names[0];
  for (unsigned int i = 0; i < _pp_num; ++i)
  {
    if (i > 0) variables += "," + _pp_names[i];
    _pp_values[i] = &getPostprocessorValueByName(_pp_names[i]);
  }

  // build the expression object
  if (_fp.Parse(_expression, variables) >= 0)
     mooseError(std::string("Invalid function\n" + _expression + "\nin Terminator.\n") + _fp.ErrorMsg());

  _params = new Real[_pp_num];
}

Terminator::~Terminator()
{
  delete[] _params;
}

void
Terminator::execute()
{
  // copy current Postprocessor values into the FParser parameter buffer
  for (unsigned int i = 0; i < _pp_num; ++i)
    _params[i] = *(_pp_values[i]);

  // request termination of the run in case the expression evaluates to true
  if (_fp.Eval(_params) != 0)
    _fe_problem.getMooseApp().getExecutioner()->terminate();
}
