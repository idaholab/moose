//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReducedPressurePIMPLE.h"
#include "FEProblem.h"
#include "AuxiliarySystem.h"
#include "LinearSystem.h"

registerMooseObject("NavierStokesApp", ReducedPressurePIMPLE);

InputParameters
ReducedPressurePIMPLE::validParams()
{
  InputParameters params = TransientBase::validParams();
  params.addClassDescription(
      "Solves the transient Navier-Stokes equations using a reduced-pressure PIMPLE algorithm with "
      "diffuse-interface face-flux hooks and linear finite volume variables.");
  params += ReducedPressurePIMPLESolve::validParams();

  return params;
}

ReducedPressurePIMPLE::ReducedPressurePIMPLE(const InputParameters & parameters)
  : TransientBase(parameters), _solve(*this)
{
  _fixed_point_solve->setInnerSolve(_solve);
}

void
ReducedPressurePIMPLE::init()
{
  _solve.initialSetup();
  TransientBase::init();
  _solve.linkRhieChowUserObject();
  _solve.setupPressurePin();
}

void
ReducedPressurePIMPLE::takeStep(Real input_dt)
{
  TransientBase::takeStep(input_dt);

  // The face flux of an accepted step is the history the next step's transient projection and
  // first outer iteration start from
  if (lastSolveConverged())
    _solve.commitAcceptedTimestepTransportHistory();
}

Real
ReducedPressurePIMPLE::relativeSolutionDifferenceNorm(bool check_aux) const
{
  if (check_aux)
    return _aux.solution().l2_norm_diff(_aux.solutionOld()) / _aux.solution().l2_norm();
  else
  {
    // Default criterion for now until we add a "steady-state-convergence-object" option
    Real residual = 0;
    for (const auto sys : _solve.systemsToSolve())
      residual +=
          std::pow(sys->solution().l2_norm_diff(sys->solutionOld()) / sys->solution().l2_norm(), 2);
    return std::sqrt(residual);
  }
}

std::set<TimeIntegrator *>
ReducedPressurePIMPLE::getTimeIntegrators() const
{
  // We use a set because time integrators were added to every system, and we want a unique
  std::set<TimeIntegrator *> tis;
  // Get all time integrators from the systems in the FEProblemSolve
  for (const auto sys : _solve.systemsToSolve())
    for (const auto & ti : sys->getTimeIntegrators())
      tis.insert(ti.get());
  return tis;
}
