//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMControl.h"

InputParameters
THMControl::validParams()
{
  InputParameters params = Control::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  params.addPrivateParam<THMProblem *>("_thm_problem");
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

THMControl::THMControl(const InputParameters & parameters)
  : Control(parameters), _sim(getParam<THMProblem *>("_thm_problem"))
{
}
