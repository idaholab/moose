/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SolverParams.h"

SolverParams::SolverParams()
  : _type(Moose::ST_PJFNK),
    _line_search(Moose::LS_INVALID),
    _eigen_solve_type(Moose::EST_KRYLOVSCHUR),
    _eigen_problem_type(Moose::EPT_NON_HERMITIAN),
    _which_eigen_pairs(Moose::WEP_SMALLEST_MAGNITUDE)
{
}
