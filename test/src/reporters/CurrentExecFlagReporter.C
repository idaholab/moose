//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CurrentExecFlagReporter.h"

registerMooseObject("MooseTestApp", CurrentExecFlagReporter);

InputParameters
CurrentExecFlagReporter::validParams()
{
  return GeneralReporter::validParams();
}

CurrentExecFlagReporter::CurrentExecFlagReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _exec_flag(declareValueByName<std::string>("exec_flag", REPORTER_MODE_ROOT))
{
}

void
CurrentExecFlagReporter::execute()
{
  _exec_flag = _fe_problem.getCurrentExecuteOnFlag().name();
}
