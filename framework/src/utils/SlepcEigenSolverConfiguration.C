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

#include <slepceps.h>

/**
 * Constructur: get a reference to the \p SlepcEigenSolver variable to be able to manipulate it
 */
SlepcEigenSolverConfiguration::SlepcEigenSolverConfiguration(
    EigenProblem & eigen_problem, libMesh::SlepcEigenSolver<libMesh::Number> & slepc_eigen_solver)
  : SolverConfiguration(), _eigen_problem(eigen_problem), _slepc_solver(slepc_eigen_solver)
{
}

void
SlepcEigenSolverConfiguration::configure_solver()
{
  PetscErrorCode ierr = 0;

  if (_eigen_problem.isNonlinearEigenvalueSolver())
  {
    // Set custom monitors for SNES and KSP
    _eigen_problem.initPetscOutput();
    // Let us remove extra "eps_power" from SNES since users do not like it
    ierr = Moose::SlepcSupport::mooseSlepcEPSSNESSetUpOptionPrefix(_slepc_solver.eps());
    LIBMESH_CHKERR(ierr);
    // Let us hook up a customize PC if users ask. Users still can use PETSc options to override
    // this setting
    if (_eigen_problem.solverParams()._customized_pc_for_eigen)
    {
      ierr = Moose::SlepcSupport::mooseSlepcEPSSNESSetCustomizePC(_slepc_solver.eps());
      LIBMESH_CHKERR(ierr);
    }
    // Let set a default PC side. I would like to have the setting be consistent with
    // what we do in regular nonlinear executioner. Petsc options are able to override
    // this setting
    Moose::SlepcSupport::mooseSlepcEPSSNESKSPSetPCSide(_eigen_problem, _slepc_solver.eps());
    // A customized stopping test for nonlinear free power iterations.
    // Nonlinear power iterations need to be marked as converged in EPS to
    // retrieve solution from SLEPc EPS.
    ierr = EPSSetStoppingTestFunction(
        _slepc_solver.eps(), Moose::SlepcSupport::mooseSlepcStoppingTest, &_eigen_problem, NULL);
    LIBMESH_CHKERR(ierr);

    // Remove all SLEPc monitors.
    ierr = EPSMonitorCancel(_slepc_solver.eps());
    LIBMESH_CHKERR(ierr);
    // A customized EPS monitor in moose. We need to print only eigenvalue
    ierr = EPSMonitorSet(
        _slepc_solver.eps(), Moose::SlepcSupport::mooseSlepcEPSMonitor, &_eigen_problem, NULL);
    LIBMESH_CHKERR(ierr);
  }
}

#endif
