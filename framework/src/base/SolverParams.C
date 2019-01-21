//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolverParams.h"

SolverParams::SolverParams()
  : _type(Moose::ST_PJFNK),
    _line_search(Moose::LS_INVALID),
    _mffd_type(Moose::MFFD_INVALID),
    _eigen_solve_type(Moose::EST_KRYLOVSCHUR),
    _eigen_problem_type(Moose::EPT_SLEPC_DEFAULT),
    _which_eigen_pairs(Moose::WEP_SLEPC_DEFAULT),
    _has_picard_its(false),
    _picard_max_its(1),
    _picard_rel_tol(1e-8),
    _picard_abs_tol(1e-50),
    _picard_force_norms(false),
    _picard_relaxation_factor(1),
    _picard_self_relaxation_factor(1),
    _max_xfem_update(std::numeric_limits<unsigned int>::max()),
    _update_xfem_at_timestep_begin(false)
{
}
