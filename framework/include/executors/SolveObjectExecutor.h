//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executor.h"

/**
 * Runs the solve of a SolveObject of type T. The executor owns the solve object, which reads the
 * parameters of the executor. The executor fails if the solve does not converge.
 *
 * An application registers each instantiation that it needs through a type alias, for example:
 *
 *   using FooSolveExecutor = SolveObjectExecutor<FooSolve>;
 *   registerMooseObject("FooApp", FooSolveExecutor);
 */
template <typename T>
class SolveObjectExecutor : public Executor
{
public:
  static InputParameters validParams();

  SolveObjectExecutor(const InputParameters & parameters);

  virtual void initialSetup() override { _solve_object.initialSetup(); }

protected:
  virtual Result run() override;

  /// The solve object that this executor runs
  T _solve_object;
};

template <typename T>
InputParameters
SolveObjectExecutor<T>::validParams()
{
  const InputParameters solve_params = T::validParams();
  InputParameters params = Executor::validParams();
  params += solve_params;
  // InputParameters::operator+= does not copy the class description
  const auto & description = solve_params.getClassDescription();
  params.addClassDescription(description.empty() ? "Runs the solve of a solve object"
                                                 : description);
  return params;
}

template <typename T>
SolveObjectExecutor<T>::SolveObjectExecutor(const InputParameters & parameters)
  : Executor(parameters), _solve_object(*this)
{
}

template <typename T>
Executor::Result
SolveObjectExecutor<T>::run()
{
  Result & result = newResult();
  if (!_solve_object.solve())
    result.fail("the solve did not converge");
  return result;
}
