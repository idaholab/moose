//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEToNEML2.h"

InputParameters
MOOSEToNEML2::validParams()
{
  auto params = emptyInputParameters();
  params.addRequiredParam<std::string>(
      "to_neml2", NEML2Utils::docstring("Name of the NEML2 variable or parameter to write to"));
  return params;
}

#ifndef NEML2_ENABLED

MOOSEToNEML2::MOOSEToNEML2(const InputParameters & /*params*/) {}

#else

MOOSEToNEML2::MOOSEToNEML2(const InputParameters & params)
  : _mode(Mode::UNDEFINED), _raw_name(params.get<std::string>("to_neml2"))
{
  NEML2Utils::assertNEML2Enabled();
}

void
MOOSEToNEML2::setMode(MOOSEToNEML2::Mode m) const
{
  _mode = m;

  if (_mode == Mode::VARIABLE || _mode == Mode::OLD_VARIABLE)
    _neml2_variable = NEML2Utils::parseVariableName(_raw_name);
  else if (_mode == Mode::PARAMETER)
    _neml2_parameter = _raw_name;
  else
    mooseError("Encountered invalid Mode in MOOSEToNEML2::setMode");

  checkMode();
}

void
MOOSEToNEML2::checkMode() const
{
  if (_mode == Mode::VARIABLE)
    NEML2Utils::assertVariable(_neml2_variable);
  if (_mode == Mode::OLD_VARIABLE)
    NEML2Utils::assertOldVariable(_neml2_variable);
}

const neml2::VariableName &
MOOSEToNEML2::NEML2VariableName() const
{
  mooseAssert(_mode == Mode::VARIABLE || _mode == Mode::OLD_VARIABLE,
              "Mode must be VARIABLE or OLD_VARIABLE when calling NEML2Variable");
  return _neml2_variable;
}

const std::string &
MOOSEToNEML2::NEML2ParameterName() const
{
  mooseAssert(_mode == Mode::PARAMETER, "Mode must be PARAMETER when calling NEML2Parameter");
  return _neml2_parameter;
}

void
MOOSEToNEML2::insertInto(neml2::ValueMap & input,
                         std::map<std::string, neml2::Tensor> & params) const
{
  if (_mode == Mode::VARIABLE || _mode == Mode::OLD_VARIABLE)
    input[_neml2_variable] = gatheredData();
  else if (_mode == Mode::PARAMETER)
    params[_neml2_parameter] = gatheredData();
  else
    mooseError("Encountered invalid Mode in MOOSEToNEML2::insertInto");
}
#endif
