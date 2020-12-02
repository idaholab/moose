//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCopyInitialSolution.h"
#include "NonlinearSystem.h"

registerMooseObject("MooseTestApp", TestCopyInitialSolution);

InputParameters
TestCopyInitialSolution::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  return params;
}

TestCopyInitialSolution::TestCopyInitialSolution(const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
  _fe_problem.getNonlinearSystemBase().needSolutionState(2);
}

TestCopyInitialSolution::~TestCopyInitialSolution() {}

void
TestCopyInitialSolution::initialize()
{
}

void
TestCopyInitialSolution::execute()
{
  // Get References to the solution
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  NumericVector<Number> & soln = nl.solution();
  NumericVector<Number> & soln_old = nl.solutionOld();
  NumericVector<Number> & soln_older = nl.solutionOlder();

  // Perform the comparisions
  Number a = soln.compare(soln_old);
  Number b = soln.compare(soln_older);

  // Set the flag for tripping the errors in the getValue method
  if (a == -1 && b == -1)
    _value = true;
  else
    _value = false;
}

Real
TestCopyInitialSolution::getValue()
{
  if (_value)
    mooseError("Solutions are equal, test passed!");
  else
    mooseError("Solutions are not equal, the test failed!");

  return 0.0;
}
