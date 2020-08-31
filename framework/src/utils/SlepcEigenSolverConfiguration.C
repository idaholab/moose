//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html


#include "SlepcEigenSolverConfiguration.h"
#include "SlepcSupport.h"

#include <slepceps.h>

/**
 * Constructur: get a reference to the \p SlepcEigenSolver variable to be able to manipulate it
 */
SlepcEigenSolverConfiguration::SlepcEigenSolverConfiguration(EigenProblem & eigen_problem, libMesh::SlepcEigenSolver<libMesh::Number> &slepc_eigen_solver):
  SolverConfiguration(),
  _eigen_problem(eigen_problem),
  _slepc_solver(slepc_eigen_solver)
{}


void
SlepcEigenSolverConfiguration::configure_solver()
{
  PetscErrorCode ierr;

  std::cout<<" configure_solver "<<std::endl;

  ierr = EPSSetStoppingTestFunction(_slepc_solver.eps(),Moose::SlepcSupport::mooseSlepcStoppingTest,&_eigen_problem,NULL);
  LIBMESH_CHKERR(ierr);
}
