#include "OptimizeSolve.h"
#include "IsopodAppTypes.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

InputParameters
OptimizeSolve::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addRequiredParam<std::string>("form_function", "Form function object.");
  params.addParam<std::string>(
      "tao_options", "", "Command line parameters for PETSc/tao optimization.");
  ExecFlagEnum exec_enum = ExecFlagEnum();
  exec_enum.addAvailableFlags(EXEC_NONE, EXEC_OBJECTIVE, EXEC_GRADIENT, EXEC_HESSIAN);
  exec_enum = {EXEC_OBJECTIVE, EXEC_GRADIENT, EXEC_HESSIAN};
  params.addParam<ExecFlagEnum>(
      "solve_on", exec_enum, "List of flags indicated when inner system solve should occur.");
  return params;
}

OptimizeSolve::OptimizeSolve(Executioner * ex)
  : SolveObject(ex),
    _tao_options(getParam<std::string>("tao_options")),
    _solve_on(getParam<ExecFlagEnum>("solve_on").items())
{
}

bool
OptimizeSolve::solve()
{
  // Grab form function
  auto ff_name = getParam<std::string>("form_function");
  std::vector<FormFunction *> ffs;
  _problem.theWarehouse()
      .query()
      .condition<AttribName>(ff_name)
      .condition<AttribSystem>("FormFunction")
      .queryInto(ffs);
  if (ffs.empty())
    mooseError("Unable to find a FormFunction object with the name '" + ff_name + "'");
  _form_function = ffs[0];

  // Petsc error code to be checked after each petsc call
  PetscErrorCode ierr;

  // Initialize tao object
  ierr = TaoCreate(_communicator.get(), &_tao);
  CHKERRQ(ierr);

  // Set input parameters
  ierr = PetscOptionsInsertString(NULL, _tao_options.c_str());
  CHKERRQ(ierr);

  // Set solve type
  ierr = TaoSetType(_tao, TAONTR);
  CHKERRQ(ierr);

  // Set initial guess
  ierr = TaoSetInitialVector(_tao, _form_function->getParameters().vec());
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

  // Solve system
  ierr = TaoSolve(_tao);
  CHKERRQ(ierr);

  ierr = TaoDestroy(&_tao);
  CHKERRQ(ierr);

  return true;
}

PetscErrorCode
OptimizeSolve::objectiveFunctionWrapper(Tao /*tao*/, Vec x, Real * objective, void * ctx)
{
  auto * solver = static_cast<OptimizeSolve *>(ctx);
  libMesh::PetscVector<Number> param(x, solver->getMooseApp().comm());
  (*objective) = solver->objectiveFunction(param);
  return 0;
}

PetscErrorCode
OptimizeSolve::gradientFunctionWrapper(Tao /*tao*/, Vec x, Vec gradient, void * ctx)
{
  auto * solver = static_cast<OptimizeSolve *>(ctx);
  libMesh::PetscVector<Number> param(x, solver->getMooseApp().comm());
  libMesh::PetscVector<Number> grad(gradient, solver->getMooseApp().comm());
  solver->gradientFunction(param, grad);
  return 0;
}

PetscErrorCode
OptimizeSolve::hessianFunctionWrapper(Tao /*tao*/, Vec x, Mat hessian, Mat /*pc*/, void * ctx)
{
  auto * solver = static_cast<OptimizeSolve *>(ctx);
  libMesh::PetscVector<Number> param(x, solver->getMooseApp().comm());
  libMesh::PetscMatrix<Number> mat(hessian, solver->getMooseApp().comm());
  solver->hessianFunction(param, mat);
  return 0;
}

Real
OptimizeSolve::objectiveFunction(const libMesh::PetscVector<Number> & x)
{
  _form_function->setParameters(x);
  _problem.execute(EXEC_OBJECTIVE);
  _problem.execMultiApps(EXEC_OBJECTIVE);
  if (_solve_on.find(EXEC_OBJECTIVE) != _solve_on.end())
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
  if (_solve_on.find(EXEC_GRADIENT) != _solve_on.end())
    _inner_solve->solve();
  _form_function->computeGradient();
  gradient.swap(_form_function->getGradient());
}

void
OptimizeSolve::hessianFunction(const libMesh::PetscVector<Number> & x,
                               libMesh::PetscMatrix<Number> & hessian)
{
  _form_function->setParameters(x);
  _problem.execute(EXEC_HESSIAN);
  _problem.execMultiApps(EXEC_HESSIAN);
  if (_solve_on.find(EXEC_HESSIAN) != _solve_on.end())
    _inner_solve->solve();
  _form_function->computeHessian();
  hessian.swap(_form_function->getHessian());
}
