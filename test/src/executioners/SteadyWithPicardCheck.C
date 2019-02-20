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

template <>
InputParameters
validParams<SteadyWithPicardCheck>()
{
  InputParameters params = validParams<Steady>();

  params.addRequiredParam<PostprocessorName>(
      "pp_name", "Postprocessor used to control the Picard convergence");
  params.addParam<Real>("pp_step_tol", 1e-6, "Relative step tolerance on the postprocessor");
  return params;
}

SteadyWithPicardCheck::SteadyWithPicardCheck(const InputParameters & parameters)
  : Steady(parameters), _pp_step_tol(getParam<Real>("pp_step_tol"))
{
}

void
SteadyWithPicardCheck::init()
{
  _pp_value = &getPostprocessorValue("pp_name");
  Steady::init();
}

bool
SteadyWithPicardCheck::augmentedPicardConvergenceCheck() const
{
  Real rel_err = std::abs((*_pp_value - _pp_value_old) / *_pp_value);

  std::ostringstream os;
  os << std::setprecision(10);
  os << " Old: " << _pp_value_old << " New: " << *_pp_value;
  os << std::scientific << std::setprecision(3) << " Error: " << rel_err;
  _console << COLOR_MAGENTA << os.str() << COLOR_DEFAULT << std::endl;
  if (rel_err < _pp_step_tol)
    return true;
  else
    return false;
}

void
SteadyWithPicardCheck::preSolve()
{
  Steady::preSolve();
  _pp_value_old = *_pp_value;
}
