//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterDebugOutput.h"

// MOOSE includes
#include "FEProblemBase.h"

registerMooseObject("MooseApp", ReporterDebugOutput);

InputParameters
ReporterDebugOutput::validParams()
{
  InputParameters params = Output::validParams();
  params.addClassDescription("Debug output object for displaying Reporter information.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  return params;
}

ReporterDebugOutput::ReporterDebugOutput(const InputParameters & parameters) : Output(parameters) {}

void
ReporterDebugOutput::output(const ExecFlagType & /*type*/)
{
  _console << "\nDeclared/requested Reporter Information:\n\n  "
           << MooseUtils::replaceAll(
                  _problem_ptr->getReporterData().getReporterInfo(), "\n", "\n  ")
           << std::endl;
}
