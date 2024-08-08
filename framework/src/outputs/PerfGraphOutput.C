//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "PerfGraphOutput.h"
#include "MooseApp.h"
#include "MooseObjectParameterName.h"
#include "InputParameterWarehouse.h"
#include "ConsoleUtils.h"

registerMooseObject("MooseApp", PerfGraphOutput);

InputParameters
PerfGraphOutput::validParams()
{
  // Get the base class parameters
  InputParameters params = Output::validParams();

  // Hide/show variable output options
  params.addParam<std::vector<VariableName>>(
      "hide",
      "A list of the variables and postprocessors that should NOT be output to the"
      "file (may include Variables, ScalarVariables, and Postprocessor names).");

  params.set<ExecFlagEnum>("execute_on") = {EXEC_FINAL};

  params.addParam<unsigned int>(
      "level", 1, "The level of detail to output.  Higher levels will yield more detail.");

  params.addParam<bool>("heaviest_branch",
                        false,
                        "Whether or not to print out the trace through the code that took the "
                        "longest amount of time");

  params.addParam<unsigned int>("heaviest_sections",
                                0,
                                "The number of sections to print out showing the parts of the code "
                                "that take the most time.  When '0' it won't print at all.");

  params.addClassDescription("Controls output of the PerfGraph: the performance log for MOOSE");

  // Return the InputParameters
  return params;
}

PerfGraphOutput::PerfGraphOutput(const InputParameters & parameters)
  : Output(parameters),
    _level(getParam<unsigned int>("level")),
    _heaviest_branch(getParam<bool>("heaviest_branch")),
    _heaviest_sections(getParam<unsigned int>("heaviest_sections"))
{
}

bool
PerfGraphOutput::shouldOutput()
{
  // We don't want the Perflog to get dumped at odd times. Ignore the FORCED flag.
  return _execute_on.isValueSet(_current_execute_flag);
}

void
PerfGraphOutput::output()
{
  if (!_app.getParam<bool>("no_timing"))
  {
    _console << '\n';

    perfGraph().print(_console, _level);

    if (_heaviest_branch)
      perfGraph().printHeaviestBranch(_console);

    if (_heaviest_sections)
      perfGraph().printHeaviestSections(_console, _heaviest_sections);

    _console << std::flush;
  }
}
