//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestFEProblemSolve.h"

#include "MooseTestAppTypes.h"
#include "FEProblemBase.h"

InputParameters
TestFEProblemSolve::validParams()
{
  InputParameters params = FEProblemSolve::validParams();
  return params;
}

TestFEProblemSolve::TestFEProblemSolve(const InputParameters & parameters)
  : FEProblemSolve(parameters)
{
}

bool
TestFEProblemSolve::solve()
{
  bool converged = FEProblemSolve::solve();

  if (converged)
  {
    _problem.execute(EXEC_JUST_GO);
    _problem.execute(EXEC_JUST_GO);
    _problem.execute(EXEC_JUST_GO);
    _problem.execute(EXEC_JUST_GO);
  }

  return converged;
}
