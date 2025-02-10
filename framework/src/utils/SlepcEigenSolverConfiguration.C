//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_SLEPC

#include "SlepcEigenSolverConfiguration.h"
#include "SlepcSupport.h"
#include "EigenProblem.h"
#include "NonlinearEigenSystem.h"
#include "libmesh/slepc_eigen_solver.h"
#include <slepceps.h>

/**
 * Constructor: get a reference to the \p SlepcEigenSolver variable to be able to manipulate it
 */
SlepcEigenSolverConfiguration::SlepcEigenSolverConfiguration(
    EigenProblem & eigen_problem,
    libMesh::SlepcEigenSolver<libMesh::Number> & slepc_eigen_solver,
    const NonlinearEigenSystem & nl_eigen_sys)
  : SolverConfiguration(),
    _eigen_problem(eigen_problem),
    _slepc_solver(slepc_eigen_solver),
    _nl_eigen_sys(nl_eigen_sys)
{
}

void
SlepcEigenSolverConfiguration::configure_solver()
{
  if (_eigen_problem.isNonlinearEigenvalueSolver(_nl_eigen_sys.number()))
  {
    // Set custom monitors for SNES and KSP
    _eigen_problem.initPetscOutputAndSomeSolverSettings();
    // Let us remove extra "eps_power" from SNES since users do not like it
    LibmeshPetscCallA(_eigen_problem.comm().get(),
                      Moose::SlepcSupport::mooseSlepcEPSSNESSetUpOptionPrefix(_slepc_solver.eps()));
    // Let us hook up a customize PC if users ask. Users still can use PETSc options to override
    // this setting
    if (_eigen_problem.solverParams(_nl_eigen_sys.number())._customized_pc_for_eigen)
      LibmeshPetscCallA(_eigen_problem.comm().get(),
                        Moose::SlepcSupport::mooseSlepcEPSSNESSetCustomizePC(_slepc_solver.eps()));
    // Let set a default PC side. I would like to have the setting be consistent with
    // what we do in regular nonlinear executioner. Petsc options are able to override
    // this setting
    LibmeshPetscCallA(
        _eigen_problem.comm().get(),
        Moose::SlepcSupport::mooseSlepcEPSSNESKSPSetPCSide(_eigen_problem, _slepc_solver.eps()));
    // A customized stopping test for nonlinear free power iterations.
    // Nonlinear power iterations need to be marked as converged in EPS to
    // retrieve solution from SLEPc EPS.
    LibmeshPetscCallA(_eigen_problem.comm().get(),
                      EPSSetStoppingTestFunction(_slepc_solver.eps(),
                                                 Moose::SlepcSupport::mooseSlepcStoppingTest,
                                                 &_eigen_problem,
                                                 NULL));

    // Remove all SLEPc monitors.
    LibmeshPetscCallA(_eigen_problem.comm().get(), EPSMonitorCancel(_slepc_solver.eps()));
    // A customized EPS monitor in moose. We need to print only eigenvalue
    LibmeshPetscCallA(
        _eigen_problem.comm().get(),
        EPSMonitorSet(
            _slepc_solver.eps(), Moose::SlepcSupport::mooseSlepcEPSMonitor, &_eigen_problem, NULL));
  }
}

#endif
