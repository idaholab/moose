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

#ifndef SOLVERPARAMS_H
#define SOLVERPARAMS_H

#include "MooseTypes.h"

class SolverParams
{
public:
  SolverParams();

  Moose::SolveType _type;
  Moose::LineSearchType _line_search;

  // solver parameters for eigenvalue problems
  Moose::EigenSolveType _eigen_solve_type;
  Moose::EigenProblemType _eigen_problem_type;
  Moose::WhichEigenPairs _which_eigen_pairs;
};

#endif /* SOLVERPARAMS_H */
