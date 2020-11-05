//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_SLEPC

#include "EigenProblem.h"

#include "libmesh/solver_configuration.h"
#include "libmesh/slepc_eigen_solver.h"

class SlepcEigenSolverConfiguration : public libMesh::SolverConfiguration
{
public:
  /**
   * Constructur: get a reference to the \p SlepcEigenSolver variable to be able to manipulate it
   */
  SlepcEigenSolverConfiguration(EigenProblem & eigen_problem,
                                libMesh::SlepcEigenSolver<libMesh::Number> & slepc_eigen_solver);

  virtual void configure_solver() override;

private:
  EigenProblem & _eigen_problem;

  /**
   *The slepc eigen solver object that we are configuring
   */
  libMesh::SlepcEigenSolver<libMesh::Number> & _slepc_solver;
};

#endif
