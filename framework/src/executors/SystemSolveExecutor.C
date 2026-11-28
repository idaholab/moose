//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SystemSolveExecutor.h"
#include "FEProblemBase.h"
#include "FEProblemSolve.h"
#include "NonlinearSystemBase.h"
#include "LinearSystem.h"
#include "Convergence.h"
#include "ConvergenceIterationTypes.h"
#include "PetscSupport.h"

registerMooseObject("MooseApp", SystemSolveExecutor);

InputParameters
SystemSolveExecutor::validParams()
{
  InputParameters params = Executor::validParams();
  params.addClassDescription("Solves one solver system, nonlinear or linear");
  params.addRequiredParam<SolverSystemName>("system", "The solver system to solve");
  params += Moose::PetscSupport::flagAndPairOptions();
  params += Moose::PetscSupport::newtonKrylovParams();
  params += Moose::PetscSupport::kspRelatedParams();
  params += FEProblemSolve::mooseLineSearchParams();
  params.addParam<ConvergenceName>(
      "convergence",
      "The Convergence object of the system solve: nonlinear for a nonlinear system, linear for a "
      "linear system. Without it, a nonlinear system uses the default nonlinear Convergence, and a "
      "linear system uses the linear solver tolerances.");
  // Executioner::validParams adds the MultiApp fixed point relaxation factor, which an Executor
  // does not use. Here it relaxes the solution of the system.
  params.setDocString("relaxation_factor",
                      "Fraction of the new solution that a converged solve keeps. The rest comes "
                      "from the solution of the previous multi-system fixed point iteration.");

  // The label of the status line includes the system name. The default is true if the problem
  // has more than one solver system.
  params.addPrivateParam<bool>("_print_system_name");
  // If true, the solver object that created this executor sets the problem-wide linear solver
  // tolerances. The executor then sets on its system only the tolerances that the user set.
  params.addPrivateParam<bool>("_problem_wide_linear_tolerances", false);

  params.addParamNamesToGroup("l_tol l_abs_tol l_max_its", "Linear Solver");
  return params;
}

SystemSolveExecutor::SystemSolveExecutor(const InputParameters & parameters)
  : Executor(parameters),
    _sys(_fe_problem.getSolverSystem(
        _fe_problem.solverSysNum(getParam<SolverSystemName>("system")))),
    _is_nonlinear(dynamic_cast<NonlinearSystemBase *>(&_sys)),
    _relaxation_factor(getParam<Real>("relaxation_factor")),
    _solve_name((isParamValid("_print_system_name") ? getParam<bool>("_print_system_name")
                                                    : _fe_problem.numSolverSystems() > 1)
                    ? "System " + _sys.name() + ": Solve"
                    : " Solve")
{
  if (!_is_nonlinear)
    for (const auto & param : Moose::PetscSupport::newtonKrylovParams().getParametersList())
      if (isParamSetByUser(param))
        paramError(param, "This parameter applies only to nonlinear systems");

  // The line search object is problem-wide. FEProblemSolve adds it for the Executioner.
  if (_is_nonlinear &&
      FEProblemSolve::mooseLineSearches().count(getParam<MooseEnum>("line_search")) &&
      !_fe_problem.getLineSearch())
    _fe_problem.addLineSearch(_pars);

  // Store the PETSc options. All options must be stored before the first solve, which inserts
  // them into the PETSc options database.
  const auto prefix = _sys.prefix();
  if (!_is_nonlinear)
    Moose::PetscSupport::dontAddCommonSNESOptions(_fe_problem, prefix);
  Moose::PetscSupport::storePetscOptions(_fe_problem, _sys, *this);
  Moose::PetscSupport::setConvergedReasonFlags(_fe_problem, prefix);
  auto & solver_params = _fe_problem.solverParams(_sys.number());
  solver_params._prefix = prefix;
  solver_params._solver_sys_num = _sys.number();

  // Linear solver tolerances on the system take precedence over the EquationSystems ones, see
  // libMesh::ImplicitSystem::get_linear_solve_parameters
  const bool problem_wide_tolerances = getParam<bool>("_problem_wide_linear_tolerances");
  auto & sys_params = _sys.system().parameters;
  if (!problem_wide_tolerances || isParamSetByUser("l_tol"))
    sys_params.set<Real>("linear solver tolerance") = getParam<Real>("l_tol");
  if (!problem_wide_tolerances || isParamSetByUser("l_abs_tol"))
    sys_params.set<Real>("linear solver absolute tolerance") = getParam<Real>("l_abs_tol");
  if (!problem_wide_tolerances || isParamSetByUser("l_max_its"))
    sys_params.set<unsigned int>("linear solver maximum iterations") =
        getParam<unsigned int>("l_max_its");

  // Set the Convergence object of the system
  if (_is_nonlinear)
  {
    if (isParamValid("convergence"))
    {
      if (_fe_problem.onlyAllowDefaultNonlinearConvergence())
        paramError("convergence", "The selected problem does not allow a nonlinear Convergence");
      _fe_problem.setNonlinearConvergence(_sys.name(), getParam<ConvergenceName>("convergence"));
    }
    else if (!_fe_problem.hasNonlinearConvergenceName(_sys.name()))
      _fe_problem.setNeedToAddDefaultNonlinearConvergence();
  }
  else if (isParamValid("convergence"))
    _fe_problem.setLinearConvergence(_sys.name(), getParam<ConvergenceName>("convergence"));

  // Relaxation needs the solution of the previous multi-system fixed point iteration
  if (!MooseUtils::absoluteFuzzyEqual(_relaxation_factor, 1.0))
    _fe_problem.needsPreviousMultiSystemFixedPointIterationSolution(true, _sys.number());
}

void
SystemSolveExecutor::initialSetup()
{
  // Only a multi-system fixed point iteration stores the solution that relaxation needs. All
  // executors are constructed at this point, so every FixedPointExecutor has marked its inner
  // executors.
  if (!MooseUtils::absoluteFuzzyEqual(_relaxation_factor, 1.0) && !_in_fixed_point_iteration)
    paramError("relaxation_factor",
               "Relaxation requires a multi-system fixed point iteration over this executor");

  // Convergence objects exist only after the executors
  if (isParamValid("convergence"))
    _fe_problem.getConvergence(getParam<ConvergenceName>("convergence"))
        .checkIterationType(_is_nonlinear ? ConvergenceIterationTypes::NONLINEAR
                                          : ConvergenceIterationTypes::LINEAR);
}

Executor::Result
SystemSolveExecutor::run()
{
  Result & result = newResult();

  const auto sys_num = _sys.number();
  if (_is_nonlinear)
    _fe_problem.solve(sys_num);
  else
    _fe_problem.solveLinearSystem(_fe_problem.linearSysNum(_sys.name()));

  if (_fe_problem.shouldSolve())
  {
    if (_fe_problem.converged(sys_num))
    {
      _sys.applyFixedPointRelaxation(_relaxation_factor,
                                     Moose::SolutionIterationType::MultiSystemFixedPoint);

      // Store the accepted, possibly relaxed LinearFV solution for nonlinear lagging
      if (!_is_nonlinear)
        _sys.copyPreviousSolutions(Moose::SolutionIterationType::Nonlinear);

      _console << COLOR_GREEN << _solve_name << " Converged!" << COLOR_DEFAULT << "\n" << std::endl;
    }
    else
    {
      _console << COLOR_RED << _solve_name << " Did NOT Converge!" << COLOR_DEFAULT << "\n"
               << std::endl;
      result.fail("system '" + _sys.name() + "' did not converge");
      return result;
    }
  }
  else
    _console << COLOR_GREEN << _solve_name << " Skipped!" << COLOR_DEFAULT << "\n" << std::endl;

  // This is for postprocessing purposes in case none of the objects request the gradients.
  // TODO: Somehow collect information if the postprocessors need gradients and if nothing
  // needs this, just skip it
  if (!_is_nonlinear)
    static_cast<LinearSystem &>(_sys).computeGradients();

  return result;
}
