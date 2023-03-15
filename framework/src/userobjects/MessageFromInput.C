//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MessageFromInput.h"
#include "Console.h"
#include "ConsoleUtils.h"

registerMooseObject("MooseApp", MessageFromInput);

InputParameters
MessageFromInput::validParams()
{
  // Get the input parameters from the parent class
  InputParameters params = GeneralUserObject::validParams();

  // Add parameters
  params.addRequiredParam<std::string>("message", "The message to print out");

  // we run this object once at the initialization by default
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

  params.addClassDescription("Print out a message from the input file");

  return params;
}

MessageFromInput::MessageFromInput(const InputParameters & parameters)
  : GeneralUserObject(parameters), _input_message(getParam<std::string>("message"))
{
}

MessageFromInput::~MessageFromInput() {}

void
MessageFromInput::execute()
{
  // Only print out once
  if (processor_id() == 0)
  {
    auto total_width = std::setw(ConsoleUtils::console_field_width);
    _console << total_width << "\n";
    _console << "Message: " << std::endl
             << ConsoleUtils::indent(2) << COLOR_YELLOW << _input_message << "\n";
    _console << COLOR_DEFAULT << total_width << "\n" << std::endl;
    _console << std ::flush;
  }
}
