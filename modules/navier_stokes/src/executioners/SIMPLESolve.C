//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SIMPLESolve.h"
#include "FEProblem.h"

InputParameters
SIMPLESolve::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

SIMPLESolve::SIMPLESolve(Executioner & ex)
  : SolveObject(ex),
    _momentum_system_names(getParam<std::vector<SolverSystemName>>("momentum_systems")),
    _pressure_system_name(getParam<SolverSystemName>("pressure_system")),
    _pressure_sys_number(_problem.linearSysNum(_pressure_system_name)),
    _pressure_system(_problem.getLinearSystem(_pressure_sys_number))
{
  // We fetch the system numbers for the momentum components plus add vectors
  // for removing the contribution from the pressure gradient terms.
  for (auto system_i : index_range(_momentum_system_names))
  {
    _momentum_system_numbers.push_back(_problem.linearSysNum(_momentum_system_names[system_i]));
    _momentum_systems.push_back(&_problem.getLinearSystem(_momentum_system_numbers[system_i]));
  }
}

bool
SIMPLESolve::solve()
{
  // The main chunk of the code will be migrate here.
  return true;
}
