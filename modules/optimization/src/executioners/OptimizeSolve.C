#include "OptimizeSolve.h"
#include "IsopodAppTypes.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

InputParameters
OptimizeSolve::validParams()
{
  InputParameters params = emptyInputParameters();
  MooseEnum tao_solver_enum("taontr taocg taonm");
  params.addRequiredParam<MooseEnum>(
      "tao_solver", tao_solver_enum, "Tao solver to use for optimization.");
  ExecFlagEnum exec_enum = ExecFlagEnum();
  exec_enum.addAvailableFlags(EXEC_NONE, EXEC_OBJECTIVE, EXEC_GRADIENT, EXEC_HESSIAN);
  exec_enum = {EXEC_OBJECTIVE, EXEC_GRADIENT, EXEC_HESSIAN};
  params.addParam<ExecFlagEnum>(
      "solve_on", exec_enum, "List of flags indicating when inner system solve should occur.");
  return params;
}

OptimizeSolve::OptimizeSolve(Executioner * ex)
  : SolveObject(ex),
    _solve_on(getParam<ExecFlagEnum>("solve_on")),
    _tao_solver_enum(getParam<MooseEnum>("tao_solver").getEnum<TaoSolverEnum>())
{
}

bool
OptimizeSolve::solve()
{
  // Initial solve
  _inner_solve->solve();

  // Grab form function
  std::vector<FormFunction *> ffs;
  _problem.theWarehouse().query().condition<AttribSystem>("FormFunction").queryInto(ffs);
  if (ffs.empty())
    mooseError("No form function object found.");
  _form_function = ffs[0];
  _form_function->initializePetscVectors();

  // Communicator used by form function
  MPI_Comm my_comm = _form_function->getComm().get();

  // Petsc error code to be checked after each petsc call
  PetscErrorCode ierr;

  // Initialize tao object
  ierr = TaoCreate(my_comm, &_tao);
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
  ierr = TaoSetHessianRoutine(_tao,
                              _form_function->getHessian().mat(),
                              _form_function->getHessian().mat(),
                              hessianFunctionWrapper,
                              this);
  CHKERRQ(ierr);

  // Set initial guess
  ierr = TaoSetInitialVector(_tao, _form_function->getParameters().vec());
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

  return ierr == 0;
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
  libMesh::PetscVector<Number> param(x, solver->getFormFunction().getComm());
  (*objective) = solver->objectiveFunction(param);
  return 0;
}

PetscErrorCode
OptimizeSolve::gradientFunctionWrapper(Tao /*tao*/, Vec x, Vec gradient, void * ctx)
{
  auto * solver = static_cast<OptimizeSolve *>(ctx);
  libMesh::PetscVector<Number> param(x, solver->getFormFunction().getComm());
  libMesh::PetscVector<Number> grad(gradient, solver->getFormFunction().getComm());
  solver->gradientFunction(param, grad);
  return 0;
}

PetscErrorCode
OptimizeSolve::hessianFunctionWrapper(Tao /*tao*/, Vec x, Mat hessian, Mat /*pc*/, void * ctx)
{
  auto * solver = static_cast<OptimizeSolve *>(ctx);
  libMesh::PetscVector<Number> param(x, solver->getFormFunction().getComm());
  libMesh::PetscMatrix<Number> mat(hessian, solver->getFormFunction().getComm());
  solver->hessianFunction(param, mat);
  return 0;
}

Real
OptimizeSolve::objectiveFunction(const libMesh::PetscVector<Number> & x)
{
  _form_function->setParameters(x);
  // fixme lynn need to switch back to Zachs flags or something like them for adjoint and forward
  //  _problem.execute(EXEC_OBJECTIVE);
  //  _problem.execMultiApps(EXEC_OBJECTIVE);
  //  if (_solve_on.contains(EXEC_OBJECTIVE))
  _problem.execute(EXEC_NONLINEAR);
  _problem.execMultiApps(EXEC_NONLINEAR);
  if (_solve_on.contains(EXEC_NONLINEAR))
    _inner_solve->solve();
  return _form_function->computeObjective();
}

void
OptimizeSolve::gradientFunction(const libMesh::PetscVector<Number> & x,
                                libMesh::PetscVector<Number> & gradient)
{
  _form_function->setParameters(x);
  _problem.execute(EXEC_GRADIENT);
  _problem.execMultiApps(EXEC_GRADIENT);
  if (_solve_on.contains(EXEC_GRADIENT))
    _inner_solve->solve();
  _form_function->computeGradient();
  gradient = _form_function->getGradient();
}

void
OptimizeSolve::hessianFunction(const libMesh::PetscVector<Number> & x,
                               libMesh::PetscMatrix<Number> & hessian)
{
  _form_function->setParameters(x);
  _problem.execute(EXEC_HESSIAN);
  _problem.execMultiApps(EXEC_HESSIAN);
  if (_solve_on.contains(EXEC_HESSIAN))
    _inner_solve->solve();
  _form_function->computeHessian();
  hessian.swap(_form_function->getHessian());
}
