//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetReinitializationProblem.h"

registerMooseObject("LevelSetApp", LevelSetReinitializationProblem);

InputParameters
LevelSetReinitializationProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addClassDescription("A specialied problem that has a method for resetting time for level "
                             "set reinitialization execution.");
  return params;
}

LevelSetReinitializationProblem::LevelSetReinitializationProblem(const InputParameters & parameters)
  : FEProblem(parameters)
{
}

void
LevelSetReinitializationProblem::resetTime()
{
  _time = 0.0;
  _time_old = 0.0;
  _t_step = 0;
  _termination_requested = false;
}
