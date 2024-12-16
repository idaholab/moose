//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Eigenvalue.h"
#include "EigenProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearEigenSystem.h"
#include "SlepcSupport.h"
#include "UserObject.h"

#include "libmesh/petsc_solver_exception.h"

// Needed for LIBMESH_CHECK_ERR
using libMesh::PetscSolverException;

registerMooseObject("MooseApp", Eigenvalue);

InputParameters
Eigenvalue::validParams()
{
  InputParameters params = Executioner::validParams();

  params.addClassDescription(
      "Eigenvalue solves a standard/generalized linear or nonlinear eigenvalue problem");

  params += FEProblemSolve::validParams();
  params.addParam<Real>("time", 0.0, "System time");

  // matrix_free will be an invalid for griffin once the integration is done.
  // In this PR, we can not change it. It will still be a valid option when users
  // use non-Newton algorithms
  params.addParam<bool>(
      "matrix_free",
      false,
      "Whether or not to use a matrix free fashion to form operators. "
      "If true, shell matrices will be used and meanwhile a preconditioning matrix"
      "may be formed as well.");

  params.addParam<bool>(
      "precond_matrix_free",
      false,
      "Whether or not to use a matrix free fashion for forming the preconditioning matrix. "
      "If true, a shell matrix will be used for preconditioner.");

  params.addParam<bool>("constant_matrices",
                        false,
                        "Whether or not to use constant matrices so that we can use them to form "
                        "residuals on both linear and "
                        "nonlinear iterations");

  params.addParam<bool>("precond_matrix_includes_eigen",
                        false,
                        "Whether or not to include eigen kernels in the preconditioning matrix. "
                        "If true, the preconditioning matrix will include eigen kernels.");

  params.addPrivateParam<bool>("_use_eigen_value", true);

  params.addParam<Real>("initial_eigenvalue", 1, "Initial eigenvalue");
  params.addParam<PostprocessorName>(
      "normalization", "Postprocessor evaluating norm of eigenvector for normalization");
  params.addParam<Real>("normal_factor",
                        "Normalize eigenvector to make a defined norm equal to this factor");

  params.addParam<bool>("auto_initialization",
                        true,
                        "If true, we will set an initial eigen vector in moose, otherwise EPS "
                        "solver will initial eigen vector");

  params.addParamNamesToGroup("matrix_free precond_matrix_free constant_matrices "
                              "precond_matrix_includes_eigen",
                              "Matrix and Matrix-Free");
  params.addParamNamesToGroup("initial_eigenvalue auto_initialization",
                              "Eigenvector and eigenvalue initialization");
  params.addParamNamesToGroup("normalization normal_factor", "Solution normalization");

  // If Newton and Inverse Power is combined in SLEPc side
  params.addPrivateParam<bool>("_newton_inverse_power", false);

// Add slepc options and eigen problems
#ifdef LIBMESH_HAVE_SLEPC
  Moose::SlepcSupport::getSlepcValidParams(params);

  params += Moose::SlepcSupport::getSlepcEigenProblemValidParams();
#endif
  return params;
}

Eigenvalue::Eigenvalue(const InputParameters & parameters)
  : Executioner(parameters),
    _eigen_problem(*getCheckedPointerParam<EigenProblem *>(
        "_eigen_problem", "This might happen if you don't have a mesh")),
    _feproblem_solve(*this),
    _normalization(isParamValid("normalization") ? &getPostprocessorValue("normalization")
                                                 : nullptr),
    _system_time(getParam<Real>("time")),
    _time_step(_eigen_problem.timeStep()),
    _time(_eigen_problem.time()),
    _final_timer(registerTimedSection("final", 1))
{
// Extract and store SLEPc options
#ifdef LIBMESH_HAVE_SLEPC
  Moose::SlepcSupport::storeSolveType(_eigen_problem, parameters);

  Moose::SlepcSupport::setEigenProblemSolverParams(_eigen_problem, parameters);
  _eigen_problem.setEigenproblemType(_eigen_problem.solverParams()._eigen_problem_type);

  // pass two control parameters to eigen problem
  _eigen_problem.solverParams()._free_power_iterations =
      getParam<unsigned int>("free_power_iterations");
  _eigen_problem.solverParams()._extra_power_iterations =
      getParam<unsigned int>("extra_power_iterations");

  if (!isParamValid("normalization") && isParamValid("normal_factor"))
    paramError("normal_factor",
               "Cannot set scaling factor without defining normalization postprocessor.");

  _fixed_point_solve->setInnerSolve(_feproblem_solve);
  _time = _system_time;

  if (isParamValid("normalization"))
  {
    const auto & normpp = getParam<PostprocessorName>("normalization");
    if (isParamValid("normal_factor"))
      _eigen_problem.setNormalization(normpp, getParam<Real>("normal_factor"));
    else
      _eigen_problem.setNormalization(normpp);
  }

  _eigen_problem.setInitialEigenvalue(getParam<Real>("initial_eigenvalue"));

  // Set a flag to nonlinear eigen system
  _eigen_problem.getNonlinearEigenSystem(/*nl_sys_num=*/0)
      .precondMatrixIncludesEigenKernels(getParam<bool>("precond_matrix_includes_eigen"));
#else
  mooseError("SLEPc is required to use Eigenvalue executioner, please use '--download-slepc in "
             "PETSc configuration'");
#endif
  // SLEPc older than 3.13.0 can not take initial guess from moose
  // It may generate converge issues
#if PETSC_RELEASE_LESS_THAN(3, 13, 0)
  mooseDeprecated(
      "Please use SLEPc-3.13.0 or higher. Old versions of SLEPc likely produce bad convergence");
#endif
}

#ifdef LIBMESH_HAVE_SLEPC
void
Eigenvalue::init()
{
  if (isParamValid("normalization"))
  {
    const auto & normpp = getParam<PostprocessorName>("normalization");
    const auto & exec = _eigen_problem.getUserObject<UserObject>(normpp).getExecuteOnEnum();
    if (!exec.isValueSet(EXEC_LINEAR))
      mooseError("Normalization postprocessor ", normpp, " requires execute_on = 'linear'");
  }

  // Does not allow time kernels
  checkIntegrity();

  // Provide vector of ones to solver
  // "auto_initialization" is on by default and we init the vector values associated
  // with eigen-variables as ones. If "auto_initialization" is turned off by users,
  // it is up to users to provide an initial guess. If "auto_initialization" is off
  // and users does not provide an initial guess, slepc will automatically generate
  // a random vector as the initial guess. The motivation to offer this option is
  // that we have to initialize ONLY eigen variables in multiphysics simulation.
  // auto_initialization can be overriden by initial conditions.
  if (getParam<bool>("auto_initialization") && !_app.isRestarting())
    _eigen_problem.initEigenvector(1.0);

  // Some setup
  _eigen_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _eigen_problem.initialSetup();

  // Make sure all PETSc options are setup correctly
  prepareSolverOptions();
}

void
Eigenvalue::prepareSolverOptions()
{
#if PETSC_RELEASE_LESS_THAN(3, 12, 0)
  // Make sure the SLEPc options are setup for this app
  Moose::SlepcSupport::slepcSetOptions(_eigen_problem, _pars);
#else
  // Options need to be setup once only
  if (!_eigen_problem.petscOptionsInserted())
  {
    // Master app has the default data base
    if (!_app.isUltimateMaster())
      LibmeshPetscCall(PetscOptionsPush(_eigen_problem.petscOptionsDatabase()));

    Moose::SlepcSupport::slepcSetOptions(_eigen_problem, _pars);

    if (!_app.isUltimateMaster())
      LibmeshPetscCall(PetscOptionsPop());

    _eigen_problem.petscOptionsInserted() = true;
  }
#endif
}

void
Eigenvalue::checkIntegrity()
{
  // check to make sure that we don't have any time kernels in eigenvalue simulation
  if (_eigen_problem.getNonlinearSystemBase(/*nl_sys=*/0).containsTimeKernel())
    mooseError("You have specified time kernels in your eigenvalue simulation");
}

#endif

void
Eigenvalue::execute()
{
#ifdef LIBMESH_HAVE_SLEPC
  // Recovering makes sense for only transient simulations since the solution from
  // the previous time steps is required.
  if (_app.isRecovering())
  {
    _console << "\nCannot recover eigenvalue solves!\nExiting...\n" << std::endl;
    return;
  }

  // Outputs initial conditions set by users
  // It is consistent with Steady
  _time_step = 0;
  _time = _time_step;
  _eigen_problem.outputStep(EXEC_INITIAL);
  _time = _system_time;

  preExecute();

  // The following code of this function is copied from "Steady"
  // "Eigenvalue" implementation can be considered a one-time-step simulation to
  // have the code compatible with the rest moose world.
  _eigen_problem.advanceState();

  // First step in any eigenvalue state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;

#ifdef LIBMESH_ENABLE_AMR

  // Define the refinement loop
  auto steps = _eigen_problem.adaptivity().getSteps();
  for (const auto r_step : make_range(steps + 1))
  {
#endif // LIBMESH_ENABLE_AMR
    _eigen_problem.timestepSetup();

    _last_solve_converged = _fixed_point_solve->solve();
    if (!lastSolveConverged())
    {
      _console << "Aborting as solve did not converge" << std::endl;
      break;
    }

    // Compute markers and indicators only when we do have at least one adaptivity step
    if (steps)
    {
      _eigen_problem.computeIndicators();
      _eigen_problem.computeMarkers();
    }
    // need to keep _time in sync with _time_step to get correct output
    _time = _time_step;
    _eigen_problem.outputStep(EXEC_TIMESTEP_END);
    _time = _system_time;

#ifdef LIBMESH_ENABLE_AMR
    if (r_step < steps)
    {
      _eigen_problem.adaptMesh();
    }

    _time_step++;
  }
#endif

  {
    TIME_SECTION(_final_timer)
    _eigen_problem.execMultiApps(EXEC_FINAL);
    _eigen_problem.finalizeMultiApps();
    _eigen_problem.postExecute();
    _eigen_problem.execute(EXEC_FINAL);
    _time = _time_step;
    _eigen_problem.outputStep(EXEC_FINAL);
    _time = _system_time;
  }

  postExecute();

#else
  mooseError("SLEPc is required for eigenvalue executioner, please use --download-slepc when "
             "configuring PETSc ");
#endif
}
