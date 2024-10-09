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

SIMPLESolve::SIMPLESolve(Executioner & ex) : SolveObject(ex) {}

bool
SIMPLESolve::solve()
{
  // The main chunk of the code will be migrate here.
  return true;
}
