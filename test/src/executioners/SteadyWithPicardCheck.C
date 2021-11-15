//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SteadyWithPicardCheck.h"

registerMooseObject("MooseTestApp", SteadyWithPicardCheck);

InputParameters
SteadyWithPicardCheck::validParams()
{
  InputParameters params = Steady::validParams();

  params.addRequiredParam<PostprocessorName>(
      "pp_name", "Postprocessor used to control the Picard convergence");
  params.addParam<Real>("pp_step_tol", 1e-6, "Relative step tolerance on the postprocessor");
  return params;
}

SteadyWithPicardCheck::SteadyWithPicardCheck(const InputParameters & parameters)
  : Steady(parameters),
    _pp_step_tol(getParam<Real>("pp_step_tol")),
    _pp_value(getPostprocessorValue("pp_name"))
{
}

void
SteadyWithPicardCheck::preSolve()
{
  Steady::preSolve();
  _pp_value_old = _pp_value;
}
