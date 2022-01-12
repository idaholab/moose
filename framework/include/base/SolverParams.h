//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
};
