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
  if (_check_aux)
    return _aux.solution().l2_norm_diff(_aux.solutionOld()) / _aux.solution().l2_norm();
  else
  {
    // Default criterion for now until we add a "steady-state-convergence-object" option
    Real residual = 0;
    for (const auto sys : _feproblem_solve.systemsToSolve())
      residual +=
          std::pow(sys->solution().l2_norm_diff(sys->solutionOld()) / sys->solution().l2_norm(), 2);
    return std::sqrt(residual);
  }
}

std::set<TimeIntegrator *>
Transient::getTimeIntegrators() const
{
  // We use a set because time integrators were added to every system, and we want a unique
  std::set<TimeIntegrator *> tis;
  // Get all time integrators from the systems in the FEProblemSolve
  for (const auto sys : _feproblem_solve.systemsToSolve())
    for (const auto & ti : sys->getTimeIntegrators())
      tis.insert(ti.get());
  return tis;
}
