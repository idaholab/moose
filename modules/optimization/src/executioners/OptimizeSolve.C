//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizeSolve.h"
#include "OptimizationAppTypes.h"
#include "OptimizationReporterBase.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

InputParameters
OptimizeSolve::validParams()
{
  InputParameters params = emptyInputParameters();
  MooseEnum tao_solver_enum("taontr taobntr taobncg taonls taobnls taontl taobntl taolmvm "
                            "taoblmvm taonm taobqnls taoowlqn taogpcg taobmrm");
  params.addRequiredParam<MooseEnum>(
      "tao_solver", tao_solver_enum, "Tao solver to use for optimization.");
  ExecFlagEnum exec_enum = ExecFlagEnum();
  exec_enum.addAvailableFlags(EXEC_NONE,
                              OptimizationAppTypes::EXEC_FORWARD,
                              OptimizationAppTypes::EXEC_ADJOINT,
                              OptimizationAppTypes::EXEC_HOMOGENEOUS_FORWARD);
  exec_enum = {OptimizationAppTypes::EXEC_FORWARD,
               OptimizationAppTypes::EXEC_ADJOINT,
               OptimizationAppTypes::EXEC_HOMOGENEOUS_FORWARD};
  params.addParam<ExecFlagEnum>(
      "solve_on", exec_enum, "List of flags indicating when inner system solve should occur.");
  return params;
}

OptimizeSolve::OptimizeSolve(Executioner & ex)
  : SolveObject(ex),
    _my_comm(MPI_COMM_SELF),
    _solve_on(getParam<ExecFlagEnum>("solve_on")),
    _verbose(getParam<bool>("verbose")),
    _tao_solver_enum(getParam<MooseEnum>("tao_solver").getEnum<TaoSolverEnum>()),
    _parameters(std::make_unique<libMesh::PetscVector<Number>>(_my_comm))
{
  if (libMesh::n_threads() > 1)
    mooseError("OptimizeSolve does not currently support threaded execution");
}

bool
OptimizeSolve::solve()
{
  TIME_SECTION("optimizeSolve", 1, "Optimization Solve");
  // Initial solve
  _inner_solve->solve();

  // Grab objective function
  if (!_problem.hasUserObject("OptimizationReporter"))
    mooseError("No OptimizationReporter object found.");
  _obj_function = &_problem.getUserObject<OptimizationReporterBase>("OptimizationReporter");

  // Initialize solution and matrix
  _obj_function->setInitialCondition(*_parameters.get());
  _ndof = _parameters->size();
  bool solveInfo = (taoSolve() == 0);
  return solveInfo;
}

PetscErrorCode
OptimizeSolve::taoSolve()
{
  // Petsc error code to be checked after each petsc call
  PetscErrorCode ierr = 0;

  // Initialize tao object
  ierr = TaoCreate(_my_comm.get(), &_tao);
  CHKERRQ(ierr);

  TaoSetMonitor(_tao, monitor, this, nullptr);

  switch (_tao_solver_enum)
  {
    case TaoSolverEnum::NEWTON_TRUST_REGION:
      ierr = TaoSetType(_tao, TAONTR);
      break;
    case TaoSolverEnum::BOUNDED_NEWTON_TRUST_REGION:
      ierr = TaoSetType(_tao, TAOBNTR);
      break;
    case TaoSolverEnum::BOUNDED_CONJUGATE_GRADIENT:
      ierr = TaoSetType(_tao, TAOBNCG);
      break;
    case TaoSolverEnum::NEWTON_LINE_SEARCH:
      ierr = TaoSetType(_tao, TAONLS);
      break;
    case TaoSolverEnum::BOUNDED_NEWTON_LINE_SEARCH:
      ierr = TaoSetType(_tao, TAOBNLS);
      break;
    case TaoSolverEnum::NEWTON_TRUST_LINE:
      ierr = TaoSetType(_tao, TAONTL);
      break;
    case TaoSolverEnum::BOUNDED_NEWTON_TRUST_LINE:
      ierr = TaoSetType(_tao, TAOBNTL);
      break;
    case TaoSolverEnum::QUASI_NEWTON:
      ierr = TaoSetType(_tao, TAOLMVM);
      break;
    case TaoSolverEnum::BOUNDED_QUASI_NEWTON:
      ierr = TaoSetType(_tao, TAOBLMVM);
      break;

    case TaoSolverEnum::NELDER_MEAD:
      ierr = TaoSetType(_tao, TAONM);
      break;

    case TaoSolverEnum::BOUNDED_QUASI_NEWTON_LINE_SEARCH:
      ierr = TaoSetType(_tao, TAOBQNLS);
      break;
    case TaoSolverEnum::ORTHANT_QUASI_NEWTON:
      ierr = TaoSetType(_tao, TAOOWLQN);
      break;
    case TaoSolverEnum::GRADIENT_PROJECTION_CONJUGATE_GRADIENT:
      ierr = TaoSetType(_tao, TAOGPCG);
      break;
    case TaoSolverEnum::BUNDLE_RISK_MIN:
      ierr = TaoSetType(_tao, TAOBMRM);
      break;

    default:
      mooseError("Invalid Tao solve type");
  }

  CHKERRQ(ierr);
  // Set bounds for bounded optimization
  ierr = TaoSetVariableBoundsRoutine(_tao, variableBoundsWrapper, this);
  CHKERRQ(ierr);

  // Set objective and gradient functions
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
  ierr = TaoSetObjective(_tao, objectiveFunctionWrapper, this);
#else
  ierr = TaoSetObjectiveRoutine(_tao, objectiveFunctionWrapper, this);
#endif
  CHKERRQ(ierr);
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
  ierr = TaoSetObjectiveAndGradient(_tao, NULL, objectiveAndGradientFunctionWrapper, this);
#else
  ierr = TaoSetObjectiveAndGradientRoutine(_tao, objectiveAndGradientFunctionWrapper, this);
#endif
  CHKERRQ(ierr);

  // Set matrix-free version of the Hessian function
  ierr = MatCreateShell(_my_comm.get(), _ndof, _ndof, _ndof, _ndof, this, &_hessian);
  CHKERRQ(ierr);
  // Link matrix-free Hessian to Tao
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
  ierr = TaoSetHessian(_tao, _hessian, _hessian, hessianFunctionWrapper, this);
#else
  ierr = TaoSetHessianRoutine(_tao, _hessian, _hessian, hessianFunctionWrapper, this);
#endif
  CHKERRQ(ierr);

  // Set initial guess
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
  ierr = TaoSetSolution(_tao, _parameters->vec());
#else
  ierr = TaoSetInitialVector(_tao, _parameters->vec());
#endif
  CHKERRQ(ierr);

  // Set petsc options
  ierr = TaoSetFromOptions(_tao);
  CHKERRQ(ierr);

  // Backup multiapps so transient problems start with the same initial condition
  _problem.backupMultiApps(OptimizationAppTypes::EXEC_FORWARD);
  _problem.backupMultiApps(OptimizationAppTypes::EXEC_ADJOINT);
  _problem.backupMultiApps(OptimizationAppTypes::EXEC_HOMOGENEOUS_FORWARD);

  // Solve optimization
  ierr = TaoSolve(_tao);
  CHKERRQ(ierr);

  // Print solve statistics
  if (getParam<bool>("verbose"))
  {
    ierr = TaoView(_tao, PETSC_VIEWER_STDOUT_WORLD);
    CHKERRQ(ierr);
  }

  ierr = TaoDestroy(&_tao);
  CHKERRQ(ierr);

  ierr = MatDestroy(&_hessian);
  CHKERRQ(ierr);

  return ierr;
}

void
OptimizeSolve::getTaoSolutionStatus(std::vector<int> & tot_iters,
                                    std::vector<double> & gnorm,
                                    std::vector<int> & obj_iters,
                                    std::vector<double> & cnorm,
                                    std::vector<int> & grad_iters,
                                    std::vector<double> & xdiff,
                                    std::vector<int> & hess_iters,
                                    std::vector<double> & f,
                                    std::vector<int> & tot_solves) const
{
  const auto num = _total_iterate_vec.size();
  tot_iters.resize(num);
  obj_iters.resize(num);
  grad_iters.resize(num);
  hess_iters.resize(num);
  tot_solves.resize(num);
  f.resize(num);
  gnorm.resize(num);
  cnorm.resize(num);
  xdiff.resize(num);

  for (const auto i : make_range(num))
  {
    tot_iters[i] = _total_iterate_vec[i];
    obj_iters[i] = _obj_iterate_vec[i];
    grad_iters[i] = _grad_iterate_vec[i];
    hess_iters[i] = _hess_iterate_vec[i];
    tot_solves[i] = _function_solve_vec[i];
    f[i] = _f_vec[i];
    gnorm[i] = _gnorm_vec[i];
    cnorm[i] = _cnorm_vec[i];
    xdiff[i] = _xdiff_vec[i];
  }
}

void
OptimizeSolve::setTaoSolutionStatus(double f, int its, double gnorm, double cnorm, double xdiff)
{
  // set data from TAO
  _total_iterate_vec.push_back(its);
  _f_vec.push_back(f);
  _gnorm_vec.push_back(gnorm);
  _cnorm_vec.push_back(cnorm);
  _xdiff_vec.push_back(xdiff);
  // set data we collect on this optimization iteration and then reset for next iteration
  _obj_iterate_vec.push_back(_obj_iterate);
  _grad_iterate_vec.push_back(_grad_iterate);
  _hess_iterate_vec.push_back(_hess_iterate);
  // count total number of FE solves
  int solves = _obj_iterate + _grad_iterate + 2 * _hess_iterate;
  _function_solve_vec.push_back(solves);
  _obj_iterate = 0;
  _grad_iterate = 0;
  _hess_iterate = 0;
  // print verbose per iteration output
  if (_verbose)
    _console << "TAO SOLVER: iteration=" << its << "\tf=" << f << "\tgnorm=" << gnorm
             << "\tcnorm=" << cnorm << "\txdiff=" << xdiff << std::endl;
}

PetscErrorCode
OptimizeSolve::monitor(Tao tao, void * ctx)
{
  TaoConvergedReason reason;
  PetscInt its;
  PetscReal f, gnorm, cnorm, xdiff;

  TaoGetSolutionStatus(tao, &its, &f, &gnorm, &cnorm, &xdiff, &reason);
  auto * solver = static_cast<OptimizeSolve *>(ctx);
  solver->setTaoSolutionStatus((double)f, (int)its, (double)gnorm, (double)cnorm, (double)xdiff);

  return 0;
}

PetscErrorCode
OptimizeSolve::objectiveFunctionWrapper(Tao /*tao*/, Vec x, Real * objective, void * ctx)
{
  auto * solver = static_cast<OptimizeSolve *>(ctx);

  libMesh::PetscVector<Number> param(x, solver->_my_comm);
  *solver->_parameters = param;

  (*objective) = solver->objectiveFunction();
  return 0;
}

PetscErrorCode
OptimizeSolve::objectiveAndGradientFunctionWrapper(
    Tao /*tao*/, Vec x, Real * objective, Vec gradient, void * ctx)
{
  auto * solver = static_cast<OptimizeSolve *>(ctx);

  libMesh::PetscVector<Number> param(x, solver->_my_comm);
  *solver->_parameters = param;

  (*objective) = solver->objectiveFunction();

  libMesh::PetscVector<Number> grad(gradient, solver->_my_comm);

  solver->gradientFunction(grad);
  return 0;
}

PetscErrorCode
OptimizeSolve::hessianFunctionWrapper(Tao /*tao*/, Vec x, Mat /*hessian*/, Mat /*pc*/, void * ctx)
{
  // Define Hessian-vector multiplication routine
  auto * solver = static_cast<OptimizeSolve *>(ctx);
  libMesh::PetscVector<Number> param(x, solver->_my_comm);
  *solver->_parameters = param;
  PetscErrorCode ierr = MatShellSetOperation(
      solver->_hessian, MATOP_MULT, (void (*)(void))OptimizeSolve::applyHessianWrapper);
  CHKERRQ(ierr);
  return 0;
}

PetscErrorCode
OptimizeSolve::applyHessianWrapper(Mat H, Vec s, Vec Hs)
{
  void * ctx;
  MatShellGetContext(H, &ctx);
  auto * solver = static_cast<OptimizeSolve *>(ctx);
  libMesh::PetscVector<Number> sbar(s, solver->_my_comm);
  libMesh::PetscVector<Number> Hsbar(Hs, solver->_my_comm);
  return solver->applyHessian(sbar, Hsbar);
}

PetscErrorCode
OptimizeSolve::variableBoundsWrapper(Tao tao, Vec /*xl*/, Vec /*xu*/, void * ctx)
{
  auto * solver = static_cast<OptimizeSolve *>(ctx);

  PetscErrorCode ierr = solver->variableBounds(tao);
  return ierr;
}

Real
OptimizeSolve::objectiveFunction()
{
  TIME_SECTION("objectiveFunction", 2, "Objective forward solve");
  _obj_function->updateParameters(*_parameters.get());

  _problem.execute(OptimizationAppTypes::EXEC_FORWARD);

  _problem.restoreMultiApps(OptimizationAppTypes::EXEC_FORWARD);
  if (!_problem.execMultiApps(OptimizationAppTypes::EXEC_FORWARD))
    mooseError("Forward solve multiapp failed!");
  if (_solve_on.contains(OptimizationAppTypes::EXEC_FORWARD))
    _inner_solve->solve();

  _obj_iterate++;

  return _obj_function->computeObjective();
}

void
OptimizeSolve::gradientFunction(libMesh::PetscVector<Number> & gradient)
{
  TIME_SECTION("gradientFunction", 2, "Gradient adjoint solve");
  _obj_function->updateParameters(*_parameters.get());

  _problem.execute(OptimizationAppTypes::EXEC_ADJOINT);
  _problem.restoreMultiApps(OptimizationAppTypes::EXEC_ADJOINT);
  if (!_problem.execMultiApps(OptimizationAppTypes::EXEC_ADJOINT))
    mooseError("Adjoint solve multiapp failed!");
  if (_solve_on.contains(OptimizationAppTypes::EXEC_ADJOINT))
    _inner_solve->solve();

  _grad_iterate++;
  _obj_function->computeGradient(gradient);
}

PetscErrorCode
OptimizeSolve::applyHessian(libMesh::PetscVector<Number> & s, libMesh::PetscVector<Number> & Hs)
{
  TIME_SECTION("applyHessian", 2, "Hessian forward/adjoint solve");
  // What happens for material inversion when the Hessian
  // is dependent on the parameters? Deal with it later???
  // see notes on how this needs to change for Material inversion
  if (_problem.hasMultiApps() &&
      !_problem.hasMultiApps(OptimizationAppTypes::EXEC_HOMOGENEOUS_FORWARD))
    mooseError("Hessian based optimization algorithms require a sub-app with:\n"
               "   execute_on = HOMOGENEOUS_FORWARD");
  _obj_function->updateParameters(s);

  _problem.execute(OptimizationAppTypes::EXEC_HOMOGENEOUS_FORWARD);
  _problem.restoreMultiApps(OptimizationAppTypes::EXEC_HOMOGENEOUS_FORWARD);
  if (!_problem.execMultiApps(OptimizationAppTypes::EXEC_HOMOGENEOUS_FORWARD))
    mooseError("Homogeneous forward solve multiapp failed!");
  if (_solve_on.contains(OptimizationAppTypes::EXEC_HOMOGENEOUS_FORWARD))
    _inner_solve->solve();

  _obj_function->setMisfitToSimulatedValues();

  _problem.execute(OptimizationAppTypes::EXEC_ADJOINT);
  _problem.restoreMultiApps(OptimizationAppTypes::EXEC_ADJOINT);
  if (!_problem.execMultiApps(OptimizationAppTypes::EXEC_ADJOINT))
    mooseError("Adjoint solve multiapp failed!");
  if (_solve_on.contains(OptimizationAppTypes::EXEC_ADJOINT))
    _inner_solve->solve();

  _obj_function->computeGradient(Hs);
  _hess_iterate++;
  return 0;
}

PetscErrorCode
OptimizeSolve::variableBounds(Tao tao)
{
  unsigned int sz = _obj_function->getNumParams();

  libMesh::PetscVector<Number> xl(_my_comm, sz);
  libMesh::PetscVector<Number> xu(_my_comm, sz);

  // copy values from upper and lower bounds to xl and xu
  for (const auto i : make_range(sz))
  {
    xl.set(i, _obj_function->getLowerBound(i));
    xu.set(i, _obj_function->getUpperBound(i));
  }
  // set upper and lower bounds in tao solver
  PetscErrorCode ierr;
  ierr = TaoSetVariableBounds(tao, xl.vec(), xu.vec());
  return ierr;
}
