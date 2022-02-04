#include "OptimizeSolve.h"
#include "IsopodAppTypes.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

InputParameters
OptimizeSolve::validParams()
{
  InputParameters params = emptyInputParameters();
  MooseEnum tao_solver_enum("taontr taobntr taobncg taonls taobnls taontl taobntl taolmvm "
                            "taoblmvm taonm taobqnls taoowlqn taogpcg taobmrm ");
  params.addRequiredParam<MooseEnum>(
      "tao_solver", tao_solver_enum, "Tao solver to use for optimization.");
  ExecFlagEnum exec_enum = ExecFlagEnum();
  exec_enum.addAvailableFlags(EXEC_NONE, EXEC_FORWARD, EXEC_ADJOINT, EXEC_HESSIAN);
  exec_enum = {EXEC_FORWARD, EXEC_ADJOINT, EXEC_HESSIAN};
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
    _parameters(libmesh_make_unique<libMesh::PetscVector<Number>>(_my_comm))
{
  if (libMesh::n_threads() > 1)
    mooseError("OptimizeSolve does not currently support threaded execution");
}

bool
OptimizeSolve::solve()
{
  // Initial solve
  _inner_solve->solve();

  // Grab form function
  if (!_problem.hasUserObject("OptimizationReporter"))
    mooseError("No form function object found.");
  _form_function = &_problem.getUserObject<OptimizationReporter>("OptimizationReporter");

  // Initialize solution and matrix
  _form_function->setInitialCondition(*_parameters.get());
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
  ierr = TaoSetObjectiveRoutine(_tao, objectiveFunctionWrapper, this);
  CHKERRQ(ierr);
  ierr = TaoSetObjectiveAndGradientRoutine(_tao, objectiveAndGradientFunctionWrapper, this);
  CHKERRQ(ierr);

  // Set matrix-free version of the Hessian function
  ierr = MatCreateShell(_my_comm.get(), _ndof, _ndof, _ndof, _ndof, this, &_hessian);
  CHKERRQ(ierr);
  // Define Hessian-vector multiplication routine
  ierr = MatShellSetOperation(_hessian, MATOP_MULT, (void (*)(void))applyHessianWrapper);
  CHKERRQ(ierr);
  // Link matrix-free Hessian to Tao
  ierr = TaoSetHessianRoutine(_tao, _hessian, _hessian, hessianFunctionWrapper, this);
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

void
OptimizeSolve::getTaoSolutionStatus(std::vector<int> & tot_iters,
                                    std::vector<double> & gnorm,
                                    std::vector<int> & obj_iters,
                                    std::vector<double> & cnorm,
                                    std::vector<int> & grad_iters,
                                    std::vector<double> & xdiff,
                                    std::vector<int> & hess_iters,
                                    std::vector<double> & f) const
{
  size_t num = _total_iterate_vec.size();
  tot_iters.resize(num);
  obj_iters.resize(num);
  grad_iters.resize(num);
  hess_iters.resize(num);
  f.resize(num);
  gnorm.resize(num);
  cnorm.resize(num);
  xdiff.resize(num);

  for (size_t i = 0; i < num; ++i)
  {
    tot_iters[i] = _total_iterate_vec[i];
    obj_iters[i] = _obj_iterate_vec[i];
    grad_iters[i] = _grad_iterate_vec[i];
    hess_iters[i] = _hess_iterate_vec[i];
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
  // this will need to set the real parameters (Vec x) on the solver (see ln 250) so that the
  // current parameter can be used in the applyHessianWrapper.  This is not important for force
  // inversion because the hessian does not change.  This will be important for material
  // inversion.

  // everything is done by the shell matrix multiply -- applyHessianWrapper
  // This will be needed if the param needs to be set, like for material inversion.
  // For force inversion, this does not do anything.
  auto * solver = static_cast<OptimizeSolve *>(ctx);
  libMesh::PetscVector<Number> param(x, solver->_my_comm);
  *solver->_parameters = param;
  auto n = solver->_ndof;
  auto comm = solver->_my_comm.get();
  MatCreateShell(
      comm, n, n, n, n, ctx, &(solver->_hessian)); // need to fix two of the n's for parallel
  MatShellSetOperation(
      solver->_hessian, MATOP_MULT, (void (*)(void))OptimizeSolve::applyHessianWrapper);

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
OptimizeSolve::applyHessian(libMesh::PetscVector<Number> & s, libMesh::PetscVector<Number> & Hs)
{
  // What happens for material inversion when the Hessian
  // is dependent on the parameters? Deal with it later???
  // see notes on how this needs to change for Material inversion
  _form_function->updateParameters(s);
  // this might be exec_forward_hessian so 2 new multiapps for material inversion

  if (!_problem.execMultiApps(EXEC_FORWARD))
    mooseError("Forward solve multiapp failed!");
  _obj_iterate++;

  _form_function->setMisfitToSimulatedValues();
  if (!_problem.execMultiApps(EXEC_ADJOINT))
    mooseError("Adjoint solve multiapp failed!");
  _grad_iterate++;
  _form_function->computeGradient(Hs);
  _hess_iterate++;
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
  bool multiapp_passed = true;
  if (!_problem.execMultiApps(EXEC_FORWARD))
    multiapp_passed = false;

  _obj_iterate++;
  return _form_function->computeAndCheckObjective(multiapp_passed);
}

void
OptimizeSolve::gradientFunction(libMesh::PetscVector<Number> & gradient)
{
  _form_function->updateParameters(*_parameters.get());
  if (!_problem.execMultiApps(EXEC_ADJOINT))
    mooseError("Adjoint solve multiapp failed!");

  _grad_iterate++;
  _form_function->computeGradient(gradient);
}

PetscErrorCode
OptimizeSolve::variableBounds(Tao tao)
{
  // get bounds
  if (!_form_function->hasBounds())
    return 0;
  const std::vector<Real> & upper_bounds = _form_function->getUpperBounds();
  const std::vector<Real> & lower_bounds = _form_function->getLowerBounds();

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
