//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMSolvePostprocessorControl.h"

registerMooseObject("ThermalHydraulicsApp", THMSolvePostprocessorControl);

InputParameters
THMSolvePostprocessorControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The name of the postprocessot that indicates is a solve should be done.");
  params.addClassDescription("Control the solve based on a postprocessor value");
  return params;
}

THMSolvePostprocessorControl::THMSolvePostprocessorControl(const InputParameters & parameters)
  : THMControl(parameters), _solve_pps(getPostprocessorValue("postprocessor"))
{
}

void
THMSolvePostprocessorControl::execute()
{
  setControllableValueByName<bool>("Problem::" + _fe_problem.name(), "solve", _solve_pps != 0.);
}
