#include "OptimizeSolve.h"
#include "IsopodAppTypes.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

InputParameters
OptimizeSolve::validParams()
{
  InputParameters params = emptyInputParameters();
  MooseEnum tao_solver_enum("taontr taobmrm taoowlqn taolmvm taocg taonm taoblmvm taobncg taobqnls "
                            "taobnls taobntr taogpcg");
  params.addRequiredParam<MooseEnum>(
      "tao_solver", tao_solver_enum, "Tao solver to use for optimization.");
  ExecFlagEnum exec_enum = ExecFlagEnum();
  exec_enum.addAvailableFlags(EXEC_NONE, EXEC_FORWARD, EXEC_ADJOINT, EXEC_HESSIAN);
  exec_enum = {EXEC_FORWARD, EXEC_ADJOINT, EXEC_HESSIAN};
  params.addParam<ExecFlagEnum>(
      "solve_on", exec_enum, "List of flags indicating when inner system solve should occur.");
  return params;
}

OptimizeSolve::OptimizeSolve(Executioner * ex)
  : SolveObject(ex),
    _my_comm(MPI_COMM_SELF),
    _solve_on(getParam<ExecFlagEnum>("solve_on")),
    _tao_solver_enum(getParam<MooseEnum>("tao_solver").getEnum<TaoSolverEnum>()),
    _parameters(libmesh_make_unique<libMesh::PetscVector<Number>>(_my_comm)),
    _hessian(_my_comm)
{
}

bool
OptimizeSolve::solve()
{
  // Initial solve
  _inner_solve->solve();

  // Grab form function
  if (!_problem.hasUserObject("FormFunction"))
    mooseError("No form function object found.");
  _form_function = &_problem.getUserObject<FormFunction>("FormFunction");

  // Initialize solution and matrix
  _form_function->setInitialCondition(*_parameters.get());
  _ndof = _parameters->size();
  _hessian.init(/*global_rows =*/_ndof,
                /*global_cols =*/_ndof,
                /*local_rows =*/_ndof,
                /*local_cols =*/_ndof,
                /*block_diag_nz =*/_ndof,
                /*block_off_diag_nz =*/0);

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

  // Print optimization data every step
  if (getParam<bool>("verbose"))
  {
    TaoSetMonitor(_tao, monitor, nullptr, nullptr);
  }

  switch (_tao_solver_enum)
  {
    case TaoSolverEnum::NEWTON_TRUST_REGION:
      ierr = TaoSetType(_tao, TAONTR);
      break;
    case TaoSolverEnum::BUNDLE_RISK_MIN:
      ierr = TaoSetType(_tao, TAOBMRM);
      break;
    case TaoSolverEnum::ORTHANT_QUASI_NEWTON:
      ierr = TaoSetType(_tao, TAOOWLQN);
      break;
    case TaoSolverEnum::QUASI_NEWTON:
      ierr = TaoSetType(_tao, TAOLMVM);
      break;
    case TaoSolverEnum::CONJUGATE_GRADIENT:
      ierr = TaoSetType(_tao, TAOCG);
      break;
    case TaoSolverEnum::NELDER_MEAD:
      ierr = TaoSetType(_tao, TAONM);
      break;
    case TaoSolverEnum::BOUNDED_QUASI_NEWTON:
      ierr = TaoSetType(_tao, TAOBLMVM);
      break;
    case TaoSolverEnum::BOUNDED_CONJUGATE_GRADIENT:
      ierr = TaoSetType(_tao, TAOBNCG);
      break;
    case TaoSolverEnum::BOUNDED_QUASI_NEWTON_LINE_SEARCH:
      ierr = TaoSetType(_tao, TAOBQNLS);
      break;
    case TaoSolverEnum::BOUNDED_NEWTON_LINE_SEARCH:
      ierr = TaoSetType(_tao, TAOBNLS);
      break;
    case TaoSolverEnum::BOUNDED_NEWTON_TRUST_REGION:
      ierr = TaoSetType(_tao, TAOBNTR);
      break;
    case TaoSolverEnum::GRADIENT_PROJECTION_CONJUGATE_GRADIENT:
      ierr = TaoSetType(_tao, TAOGPCG);
      break;
    default:
      mooseError("Invalid Tao solve type");
  }

  CHKERRQ(ierr);

  // Set bounds for bounded optimization
  ierr = TaoSetVariableBoundsRoutine(_tao, variableBoundsWrapper, this);

  CHKERRQ(ierr);

  // Set objective, gradient, and hessian functions
  ierr = TaoSetObjectiveRoutine(_tao, objectiveFunctionWrapper, this);
  CHKERRQ(ierr);
  ierr = TaoSetObjectiveAndGradientRoutine(_tao, objectiveAndGradientFunctionWrapper, this);
  CHKERRQ(ierr);
  ierr = TaoSetHessianRoutine(_tao, _hessian.mat(), _hessian.mat(), hessianFunctionWrapper, this);
  CHKERRQ(ierr);

  // Set initial guess
  ierr = TaoSetInitialVector(_tao, _parameters->vec());
  CHKERRQ(ierr);

  // Set petsc options
  ierr = TaoSetFromOptions(_tao);
  CHKERRQ(ierr);

  // Solve optimization
  ierr = TaoSolve(_tao);
  CHKERRQ(ierr);

  // Setting data on reporter fixme lynn  this uses a custom execution flag to only call one
  // reporter,  maybe there will be more
  setTaoSolutionStatus(_tao);

  // Print solve statistics
  if (getParam<bool>("verbose"))
  {
    ierr = TaoView(_tao, PETSC_VIEWER_STDOUT_WORLD);
    CHKERRQ(ierr);
  }

  ierr = TaoDestroy(&_tao);
  CHKERRQ(ierr);

  return ierr;
}

void
OptimizeSolve::getTaoSolutionStatus(int & tot_iters,
                                    double & gnorm,
                                    int & obj_iters,
                                    double & cnorm,
                                    int & grad_iters,
                                    double & xdiff,
                                    int & hess_iters,
                                    double & f) const
{
  tot_iters = _total_iterate;
  obj_iters = _obj_iterate;
  grad_iters = _grad_iterate;
  hess_iters = _hess_iterate;
  f = _f;
  gnorm = _gnorm;
  cnorm = _cnorm;
  xdiff = _xdiff;
}

void
OptimizeSolve::setTaoSolutionStatus(Tao tao)
{
  TaoConvergedReason reason;
  PetscInt its;
  PetscReal f, gnorm, cnorm, xdiff;

  TaoGetSolutionStatus(tao, &its, &f, &gnorm, &cnorm, &xdiff, &reason);

  _total_iterate = (int)its;
  _f = double(f);
  _gnorm = double(gnorm);
  _cnorm = double(cnorm);
  _xdiff = double(xdiff);
}

PetscErrorCode
OptimizeSolve::monitor(Tao tao, void * /*ctx*/)
{
  TaoConvergedReason reason;
  PetscInt its;
  PetscReal f, gnorm, cnorm, xdiff;

  TaoGetSolutionStatus(tao, &its, &f, &gnorm, &cnorm, &xdiff, &reason);
  unsigned int print_nsteps = 1;
  if (!(its % print_nsteps))
  {
    PetscPrintf(
        PETSC_COMM_WORLD,
        "****************** TAO SOLVER OUTPUT: iteration=%D\tf=%g\tgnorm=%g\tcnorm=%g\txdiff=%g\n",
        its,
        f,
        gnorm,
        cnorm,
        xdiff);
  }
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
OptimizeSolve::hessianFunctionWrapper(Tao /*tao*/, Vec x, Mat hessian, Mat /*pc*/, void * ctx)
{
  auto * solver = static_cast<OptimizeSolve *>(ctx);

  libMesh::PetscVector<Number> param(x, solver->_my_comm);
  *solver->_parameters = param;

  libMesh::PetscMatrix<Number> mat(hessian, solver->_my_comm);

  solver->hessianFunction(mat);
  return 0;
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
  _form_function->updateParameters(*_parameters.get());

  _problem.execute(EXEC_FORWARD);
  bool multiapp_passed = true;
  if (!_problem.execMultiApps(EXEC_FORWARD))
    multiapp_passed = false;
  if (_solve_on.contains(EXEC_FORWARD))
    _inner_solve->solve();

  _obj_iterate++;
  return _form_function->computeAndCheckObjective(multiapp_passed);
}

void
OptimizeSolve::gradientFunction(libMesh::PetscVector<Number> & gradient)
{
  _form_function->updateParameters(*_parameters.get());

  _problem.execute(EXEC_ADJOINT);
  if (!_problem.execMultiApps(EXEC_ADJOINT))
    mooseError("Adjoint solve multiapp failed!");
  if (_solve_on.contains(EXEC_ADJOINT))
    _inner_solve->solve();

  _grad_iterate++;
  _form_function->computeGradient(gradient);
}

void
OptimizeSolve::hessianFunction(libMesh::PetscMatrix<Number> & hessian)
{
  _form_function->updateParameters(*_parameters.get());

  _problem.execute(EXEC_HESSIAN);
  if (!_problem.execMultiApps(EXEC_HESSIAN))
    mooseError("Hessian solve multiapp failed!");
  if (_solve_on.contains(EXEC_HESSIAN))
    _inner_solve->solve();

  _hess_iterate++;

  _form_function->computeHessian(hessian);
}

PetscErrorCode
OptimizeSolve::variableBounds(Tao tao)
{
  // get bounds
  std::vector<Real> upper_bounds = _form_function->getUpperBounds();
  std::vector<Real> lower_bounds = _form_function->getLowerBounds();

  unsigned int sz = _form_function->getNumParams();

  libMesh::PetscVector<Number> xl(_my_comm, sz);
  libMesh::PetscVector<Number> xu(_my_comm, sz);

  // copy values from upper and lower bounds to xl and xu
  for (unsigned int i = 0; i < sz; ++i)
  {
    xl.set(i, lower_bounds[i]);
    xu.set(i, upper_bounds[i]);
  }
  // set upper and lower bounds in tao solver
  PetscErrorCode ierr;
  ierr = TaoSetVariableBounds(tao, xl.vec(), xu.vec());
  return ierr;
}
