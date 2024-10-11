//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SIMPLE.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "LinearSystem.h"
#include "KernelBase.h"
#include "INSFVMomentumPressure.h"
#include "libmesh/enum_point_locator_type.h"

#include "libmesh/petsc_nonlinear_solver.h"
#include <petscerror.h>
#include <petscsys.h>
#include <petscksp.h>

registerMooseObject("NavierStokesApp", SIMPLE);

InputParameters
SIMPLE::validParams()
{
  InputParameters params = SegregatedSolverBase::validParams();

  params.addClassDescription("Solves the Navier-Stokes equations using the SIMPLE algorithm and "
                             "linear finite volume variables.");

  /*
   * We suppress parameters which are not supported yet
   */
  params.suppressParameter<SolverSystemName>("solid_energy_system");
  params.suppressParameter<std::vector<SolverSystemName>>("passive_scalar_systems");
  params.suppressParameter<std::vector<SolverSystemName>>("turbulence_systems");
  params.suppressParameter<std::vector<Real>>("passive_scalar_equation_relaxation");
  params.suppressParameter<std::vector<Real>>("turbulence_equation_relaxation");
  params.suppressParameter<MultiMooseEnum>("solid_energy_petsc_options");
  params.suppressParameter<MultiMooseEnum>("solid_energy_petsc_options_iname");
  params.suppressParameter<std::vector<std::string>>("solid_energy_petsc_options_value");
  params.suppressParameter<MultiMooseEnum>("passive_scalar_petsc_options");
  params.suppressParameter<MultiMooseEnum>("passive_scalar_petsc_options_iname");
  params.suppressParameter<std::vector<std::string>>("passive_scalar_petsc_options_value");
  params.suppressParameter<MultiMooseEnum>("turbulence_petsc_options");
  params.suppressParameter<MultiMooseEnum>("turbulence_petsc_options_iname");
  params.suppressParameter<std::vector<std::string>>("turbulence_petsc_options_value");
  params.suppressParameter<Real>("solid_energy_absolute_tolerance");
  params.suppressParameter<std::vector<Real>>("passive_scalar_absolute_tolerance");
  params.suppressParameter<std::vector<Real>>("turbulence_absolute_tolerance");
  params.suppressParameter<Real>("solid_energy_l_tol");
  params.suppressParameter<Real>("solid_energy_l_abs_tol");
  params.suppressParameter<unsigned int>("solid_energy_l_max_its");
  params.suppressParameter<Real>("passive_scalar_l_tol");
  params.suppressParameter<Real>("passive_scalar_l_abs_tol");
  params.suppressParameter<unsigned int>("passive_scalar_l_max_its");
  params.suppressParameter<Real>("turbulence_l_tol");
  params.suppressParameter<Real>("turbulence_l_abs_tol");
  params.suppressParameter<unsigned int>("turbulence_l_max_its");

  return params;
}

SIMPLE::SIMPLE(const InputParameters & parameters)
  : SegregatedSolverBase(parameters), _simple_solve(*this)
{
  _fixed_point_solve->setInnerSolve(_simple_solve);
  _time = _system_time;
}

void
SIMPLE::init()
{
  SegregatedSolverBase::init();
  _simple_solve.linkRhieChowUserObject();
  _simple_solve.setupPressurePin();
}

std::pair<unsigned int, Real>
SIMPLE::solveAdvectedSystem(const unsigned int system_num,
                            LinearSystem & system,
                            const Real relaxation_factor,
                            SolverConfiguration & solver_config,
                            const Real absolute_tol)
{
  _problem.setCurrentLinearSystem(system_num);

  // We will need some members from the implicit linear system
  LinearImplicitSystem & li_system = libMesh::cast_ref<LinearImplicitSystem &>(system.system());

  // We will need the solution, the right hand side and the matrix
  NumericVector<Number> & current_local_solution = *(li_system.current_local_solution);
  NumericVector<Number> & solution = *(li_system.solution);
  SparseMatrix<Number> & mmat = *(li_system.matrix);
  NumericVector<Number> & rhs = *(li_system.rhs);

  mmat.zero();
  rhs.zero();

  // We need a vector that stores the (diagonal_relaxed-original_diagonal) vector
  auto diff_diagonal = solution.zero_clone();

  // Fetch the linear solver from the system
  PetscLinearSolver<Real> & linear_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*li_system.get_linear_solver());

  _problem.computeLinearSystemSys(li_system, mmat, rhs, true);

  // Go and relax the system matrix and the right hand side
  relaxMatrix(mmat, relaxation_factor, *diff_diagonal);
  relaxRightHandSide(rhs, solution, *diff_diagonal);

  if (_print_fields)
  {
    _console << system.name() << " system matrix" << std::endl;
    mmat.print();
  }

  // We compute the normalization factors based on the fluxes
  Real norm_factor = computeNormalizationFactor(solution, mmat, rhs);

  // We need the non-preconditioned norm to be consistent with the norm factor
  LIBMESH_CHKERR(KSPSetNormType(linear_solver.ksp(), KSP_NORM_UNPRECONDITIONED));

  // Setting the linear tolerances and maximum iteration counts
  solver_config.real_valued_data["abs_tol"] = absolute_tol * norm_factor;
  linear_solver.set_solver_configuration(solver_config);

  // Solve the system and update current local solution
  auto its_res_pair = linear_solver.solve(mmat, mmat, solution, rhs);
  li_system.update();

  if (_print_fields)
  {
    _console << " rhs when we solve " << system.name() << std::endl;
    rhs.print();
    _console << system.name() << " solution " << std::endl;
    solution.print();
    _console << " Norm factor " << norm_factor << std::endl;
  }

  system.setSolution(current_local_solution);

  return std::make_pair(its_res_pair.first, linear_solver.get_initial_residual() / norm_factor);
}

void
SIMPLE::execute()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover SIMPLE solves!\nExiting...\n" << std::endl;
    return;
  }

  if (_problem.adaptivity().isOn())
  {
    _console << "\nCannot use SIMPLE solves with mesh adaptivity!\nExiting...\n" << std::endl;
    return;
  }

  ExecFlagEnum disabled_flags;
  disabled_flags.addAvailableFlags(EXEC_TIMESTEP_BEGIN,
                                   EXEC_TIMESTEP_END,
                                   EXEC_INITIAL,
                                   EXEC_MULTIAPP_FIXED_POINT_BEGIN,
                                   EXEC_MULTIAPP_FIXED_POINT_END,
                                   EXEC_LINEAR,
                                   EXEC_NONLINEAR);

  if (hasMultiAppError(disabled_flags))
    return;
  if (hasTransferError(disabled_flags))
    return;

#ifdef LIBMESH_ENABLE_AMR

  // Define the refinement loop
  unsigned int steps = _problem.adaptivity().getSteps();
  for (unsigned int r_step = 0; r_step <= steps; r_step++)
  {
#endif // LIBMESH_ENABLE_AMR

    _problem.timestepSetup();

    _time_step = 0;
    _problem.outputStep(EXEC_INITIAL);

    preExecute();

    _problem.advanceState();

    _time_step = 1;

    _last_solve_converged = _fixed_point_solve->solve();

    if (!lastSolveConverged())
    {
      _console << "Aborting as solve did not converge" << std::endl;
      break;
    }

    _problem.computeIndicators();
    _problem.computeMarkers();

    _time = _time_step;
    _problem.outputStep(EXEC_TIMESTEP_END);
    _time = _system_time;

#ifdef LIBMESH_ENABLE_AMR
    if (r_step != steps)
    {
      _problem.adaptMesh();
    }

    _time_step++;
  }
#endif

  {
    TIME_SECTION("final", 1, "Executing Final Objects")
    _problem.execMultiApps(EXEC_FINAL);
    _problem.finalizeMultiApps();
    _problem.postExecute();
    _problem.execute(EXEC_FINAL);
    _time = _time_step;
    _problem.outputStep(EXEC_FINAL);
    _time = _system_time;
  }

  postExecute();
}
