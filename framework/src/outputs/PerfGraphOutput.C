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

template <>
InputParameters
validParams<PerfGraphOutput>()
{
  // Get the base class parameters
  InputParameters params = validParams<Output>();
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_FINAL};

  params.addParam<unsigned int>(
      "level", 1, "The level of detail to output.  Higher levels will yield more detail.");

  params.addClassDescription("Controls output of the PerfGraph: the performance log for MOOSE");

  // Return the InputParameters
  return params;
}

PerfGraphOutput::PerfGraphOutput(const InputParameters & parameters)
  : Output(parameters), _level(getParam<unsigned int>("level"))
{
}

void
PerfGraphOutput::output(const ExecFlagType & /*type*/)
{
  _app.perfGraph().print(_console, _level);
}
