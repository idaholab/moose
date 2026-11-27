//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FixedPointExecutor.h"
#include "SystemSolveExecutor.h"
#include "FEProblemBase.h"
#include "SolverSystem.h"
#include "AuxiliarySystem.h"
#include "Convergence.h"
#include "ConvergenceIterationTypes.h"

registerMooseObject("MooseApp", FixedPointExecutor);

InputParameters
FixedPointExecutor::validParams()
{
  InputParameters params = Executor::validParams();
  params.addClassDescription(
      "Runs a multi-system fixed point iteration that sweeps over the inner executors");
  params.addRequiredParam<std::vector<ExecutorName>>(
      "inner_executors", "The executors that one sweep runs, in the order of the forward sweep");
  params.addParam<ConvergenceName>(
      "convergence",
      "The multi-system fixed point Convergence object. Without it, the executor does one sweep.");
  MooseEnum sweep_type("multiplicative symmetric_multiplicative", "multiplicative");
  params.addParam<MooseEnum>("sweep_type",
                             sweep_type,
                             "multiplicative runs the inner executors in order (1..N). "
                             "symmetric_multiplicative adds a backward sweep (1..N, N-1..1).");
  return params;
}

FixedPointExecutor::FixedPointExecutor(const InputParameters & parameters) : Executor(parameters)
{
  for (const auto & name : getParam<std::vector<ExecutorName>>("inner_executors"))
  {
    auto & inner = getExecutorByName<Executor>(name);
    _sweep.push_back(&inner);
    if (auto * const system_solve = dynamic_cast<SystemSolveExecutor *>(&inner))
      system_solve->setInFixedPointIteration();
  }

  // The backward sweep skips the last executor of the forward sweep, whose inputs have not
  // changed since it ran
  if (getParam<MooseEnum>("sweep_type") == "symmetric_multiplicative")
    for (auto i = _sweep.size(); i > 1; --i)
      _sweep.push_back(_sweep[i - 2]);
}

void
FixedPointExecutor::initialSetup()
{
  // Convergence objects exist only after the executors
  if (isParamValid("convergence"))
  {
    _convergence = &_fe_problem.getConvergence(getParam<ConvergenceName>("convergence"));
    _convergence->checkIterationType(ConvergenceIterationTypes::MULTISYSTEM_FIXED_POINT);
  }

  // Store the previous iteration of every system that needs it, for example for relaxation
  for (const auto sys_num : make_range(_fe_problem.numSolverSystems()))
    if (_fe_problem.needsPreviousMultiSystemFixedPointIterationSolution(sys_num))
    {
      auto & sys = _fe_problem.getSolverSystem(sys_num);
      sys.needSolutionState(
          1, Moose::SolutionIterationType::MultiSystemFixedPoint, sys.solution().type());
      _systems_to_copy_back.insert(&sys);
    }
  if (_fe_problem.needsPreviousMultiSystemFixedPointIterationAuxiliary())
  {
    auto & aux = _fe_problem.getAuxiliarySystem();
    aux.needSolutionState(
        1, Moose::SolutionIterationType::MultiSystemFixedPoint, aux.solution().type());
    _systems_to_copy_back.insert(&aux);
  }
}

Executor::Result
FixedPointExecutor::run()
{
  Result & result = newResult();

  for (unsigned int n_its = 1;; ++n_its)
  {
    if (_convergence)
    {
      _console << COLOR_MAGENTA << "Multi-system fixed point iteration " << n_its - 1 << ":"
               << COLOR_DEFAULT << "\n"
               << std::endl;
      _convergence->preIteration();
    }

    for (auto * const sys : _systems_to_copy_back)
      sys->copyPreviousSolutions(Moose::SolutionIterationType::MultiSystemFixedPoint);

    for (auto * const inner : _sweep)
      if (!result.record(inner->name(), inner->exec()))
        return result;

    _fe_problem.execute(EXEC_MULTISYSTEM_FIXED_POINT_ITERATION_END);
    _fe_problem.outputStep(EXEC_MULTISYSTEM_FIXED_POINT_ITERATION_END);

    if (!_convergence)
      return result;

    const auto status = _convergence->checkConvergence(n_its);
    if (status == Convergence::MooseConvergenceStatus::CONVERGED)
    {
      _console << "Multisystem iteration converged at iteration " << n_its << std::endl;
      return result;
    }
    if (status == Convergence::MooseConvergenceStatus::DIVERGED)
    {
      _console << "Multisystem iteration diverged at iteration " << n_its << std::endl;
      result.fail("diverged at iteration " + std::to_string(n_its));
      return result;
    }
  }
}
