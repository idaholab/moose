#include "OptimizeSolve.h"
#include "IsopodAppTypes.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

InputParameters
OptimizeSolve::validParams()
{
  InputParameters params = emptyInputParameters();
  MooseEnum tao_solver_enum("taontr taobmrm taoowlqn taolmvm taocg taonm");
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

  return taoSolve() == 0;
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
    default:
      mooseError("Invalid Tao solve type");
  }

  CHKERRQ(ierr);

  // Set objective, gradient, and hessian functions
  ierr = TaoSetObjectiveRoutine(_tao, objectiveFunctionWrapper, this);
  CHKERRQ(ierr);
  ierr = TaoSetGradientRoutine(_tao, gradientFunctionWrapper, this);
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

PetscErrorCode
OptimizeSolve::monitor(Tao tao, void * /*ctx*/)
{
  PetscInt its;
  PetscReal f, gnorm, cnorm, xdiff;
  TaoConvergedReason reason;

  TaoGetSolutionStatus(tao, &its, &f, &gnorm, &cnorm, &xdiff, &reason);
  unsigned int print_nsteps = 1;
  if (!(its % print_nsteps))
  {
    PetscPrintf(PETSC_COMM_WORLD,
                "****** TAO SOLVER OUTPUT: iteration=%D\tf=%g\tgnorm=%g\tcnorm=%g\txdiff=%g\n",
                its,
                (double)f,
                (double)gnorm,
                (double)cnorm,
                (double)xdiff);
  }
  return 0;
}

PetscErrorCode
OptimizeSolve::objectiveFunctionWrapper(Tao /*tao*/, Vec x, Real * objective, void * ctx)
{
  auto * solver = static_cast<OptimizeSolve *>(ctx);

  libMesh::PetscVector<Number> & param_solver =
      *cast_ptr<libMesh::PetscVector<Number> *>(solver->_parameters.get());
  libMesh::PetscVector<Number> param(x, solver->_my_comm);
  param.swap(param_solver);

  (*objective) = solver->objectiveFunction();
  return 0;
}

PetscErrorCode
OptimizeSolve::gradientFunctionWrapper(Tao /*tao*/, Vec x, Vec gradient, void * ctx)
{
  auto * solver = static_cast<OptimizeSolve *>(ctx);

  libMesh::PetscVector<Number> & param_solver =
      *cast_ptr<libMesh::PetscVector<Number> *>(solver->_parameters.get());
  libMesh::PetscVector<Number> param(x, solver->_my_comm);
  param.swap(param_solver);

  libMesh::PetscVector<Number> grad(gradient, solver->_my_comm);

  solver->gradientFunction(grad);
  return 0;
}

PetscErrorCode
OptimizeSolve::hessianFunctionWrapper(Tao /*tao*/, Vec x, Mat hessian, Mat /*pc*/, void * ctx)
{
  auto * solver = static_cast<OptimizeSolve *>(ctx);

  libMesh::PetscVector<Number> & param_solver =
      *cast_ptr<libMesh::PetscVector<Number> *>(solver->_parameters.get());
  libMesh::PetscVector<Number> param(x, solver->_my_comm);
  param.swap(param_solver);

  libMesh::PetscMatrix<Number> mat(hessian, solver->_my_comm);

  solver->hessianFunction(mat);
  return 0;
}

Real
OptimizeSolve::objectiveFunction()
{
  _form_function->updateParameters(*_parameters.get());

  _problem.execute(EXEC_FORWARD);
  _problem.execMultiApps(EXEC_FORWARD);
  if (_solve_on.contains(EXEC_FORWARD))
    _inner_solve->solve();

  return _form_function->computeObjective();
}

void
OptimizeSolve::gradientFunction(libMesh::PetscVector<Number> & gradient)
{
  _form_function->updateParameters(*_parameters.get());

  _problem.execute(EXEC_ADJOINT);
  _problem.execMultiApps(EXEC_ADJOINT);
  if (_solve_on.contains(EXEC_ADJOINT))
    _inner_solve->solve();

  _form_function->computeGradient(gradient);
}

void
OptimizeSolve::hessianFunction(libMesh::PetscMatrix<Number> & hessian)
{
  _form_function->updateParameters(*_parameters.get());

  _problem.execute(EXEC_HESSIAN);
  _problem.execMultiApps(EXEC_HESSIAN);
  if (_solve_on.contains(EXEC_HESSIAN))
    _inner_solve->solve();

  _form_function->computeHessian(hessian);
}
