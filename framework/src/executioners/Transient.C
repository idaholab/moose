//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Transient.h"

// MOOSE includes
#include "FixedPointSolve.h"
#include "AuxiliarySystem.h"
#include "SolverSystem.h"

registerMooseObject("MooseApp", Transient);

InputParameters
Transient::validParams()
{
  InputParameters params = TransientBase::validParams();
  params.addClassDescription("Executioner for time varying simulations.");

  params += FEProblemSolve::validParams();

  return params;
}

Transient::Transient(const InputParameters & parameters)
  : TransientBase(parameters), _feproblem_solve(*this)
{
  _fixed_point_solve->setInnerSolve(_feproblem_solve);
}

Real
Transient::relativeSolutionDifferenceNorm()
{
  // TODO: add multi-system support
  const NumericVector<Number> & current_solution =
      _check_aux ? _aux.solution() : *_nl.currentSolution();
  const NumericVector<Number> & old_solution = _check_aux ? _aux.solutionOld() : _nl.solutionOld();

  return current_solution.l2_norm_diff(old_solution) / current_solution.l2_norm();
}
