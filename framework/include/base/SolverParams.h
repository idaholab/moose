//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SOLVERPARAMS_H
#define SOLVERPARAMS_H

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

  // solver parameters for Picard iteration
  bool _has_picard_its;
  unsigned int _picard_max_its;
  Real _picard_rel_tol;
  Real _picard_abs_tol;
  bool _picard_force_norms;
  Real _picard_relaxation_factor;
  std::vector<std::string> _picard_relaxed_variables;

  // solver parameters for XFEM
  unsigned int _max_xfem_update;
  bool _update_xfem_at_timestep_begin;
};

#endif /* SOLVERPARAMS_H */
