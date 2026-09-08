//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

class SolverParams
{
public:
  SolverParams();

  Moose::SolveType _type;
  Moose::LineSearchType _line_search;
  Moose::MffdType _mffd_type;

  /// Whether to use the Kokkos matrix-free Jacobian-vector product (an analytic partial-assembly
  /// operator action) as both the SNES Amat and Pmat, a shell matrix whose operations are a
  /// matrix-vector product and a diagonal, which limits preconditioning to what those two supply.
  /// Mutually exclusive with the PETSc-native ST_PJFNK/ST_JFNK finite-difference matrix-free modes.
  bool _kokkos_matrix_free;

  /// Whether a p-multigrid preconditioner drives the solve's PETSc preconditioner. The hierarchy is
  /// configured level by level once the solver objects exist, so the preconditioner type a
  /// matrix-free system would otherwise default to is left to that configuration.
  bool _kokkos_p_multigrid;

  // solver parameters for eigenvalue problems
  Moose::EigenSolveType _eigen_solve_type;
  Moose::EigenProblemType _eigen_problem_type;
  Moose::WhichEigenPairs _which_eigen_pairs;
  bool _eigen_matrix_free;
  bool _eigen_matrix_vector_mult;
  bool _customized_pc_for_eigen;
  bool _precond_matrix_free;
  unsigned int _free_power_iterations;
  unsigned int _extra_power_iterations;

  // For distinguishing between multiple systems
  std::string _prefix;
  unsigned int _solver_sys_num;
};
