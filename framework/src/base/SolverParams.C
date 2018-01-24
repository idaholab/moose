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
    _which_eigen_pairs(Moose::WEP_SLEPC_DEFAULT)
{
}
