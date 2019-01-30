//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Steady.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"

#include "libmesh/equation_systems.h"

registerMooseObject("MooseApp", Steady);

template <>
InputParameters
validParams<Steady>()
{
  auto params = validParams<Transient>();

  params.set<bool>("_is_transient") = false;
  params.set<unsigned int>("num_steps") = 1;
  params.set<bool>("abort_on_solve_fail") = true;

  return params;
}

Steady::Steady(const InputParameters & parameters) : Transient(parameters)
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover steady solves!\nExiting...\n" << std::endl;
    return;
  }

  _num_steps = _problem.adaptivity().getSteps() + 1;
}
