/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "TestCopyInitialSolution.h"

template<>
InputParameters validParams<TestCopyInitialSolution>()
{
InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

TestCopyInitialSolution::TestCopyInitialSolution(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters)
{}

TestCopyInitialSolution::~TestCopyInitialSolution()
{
}

void
TestCopyInitialSolution::initialize()
{
}

void
TestCopyInitialSolution::execute()
{
  // Get References to the solution
  NonlinearSystem & nl = _fe_problem.getNonlinearSystem();
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
