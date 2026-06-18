//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "PIMPLE.h"
#include "FEProblem.h"
#include "AuxiliarySystem.h"
#include "LinearSystem.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", PIMPLE);

InputParameters
PIMPLE::validParams()
{
  InputParameters params = TransientBase::validParams();
  params.addClassDescription(
      "Solves the transient Navier-Stokes equations using the PIMPLE algorithm and "
      "linear finite volume variables.");
  params += PIMPLESolve::validParams();

  return params;
}

PIMPLE::PIMPLE(const InputParameters & parameters) : TransientBase(parameters), _pimple_solve(*this)
{
  _fixed_point_solve->setInnerSolve(_pimple_solve);
}

void
PIMPLE::init()
{
  auto & solve = pimpleSolve();
  solve.initialSetup();
  TransientBase::init();
  solve.linkRhieChowUserObject();
  solve.setupPressurePin();
}

void
PIMPLE::takeStep(Real input_dt)
{
  Real dt_to_take = input_dt;
  if (dt_to_take == -1.0)
    dt_to_take = computeConstrainedDT();

  TransientBase::takeStep(dt_to_take);
  postTakeStep();
}

Real
PIMPLE::relativeSolutionDifferenceNorm(bool check_aux) const
{
  if (check_aux)
    return _aux.solution().l2_norm_diff(_aux.solutionOld()) / _aux.solution().l2_norm();
  else
  {
    // Default criterion for now until we add a "steady-state-convergence-object" option
    Real residual = 0;
    for (const auto sys : pimpleSolve().systemsToSolve())
      residual +=
          std::pow(sys->solution().l2_norm_diff(sys->solutionOld()) / sys->solution().l2_norm(), 2);
    return std::sqrt(residual);
  }
}

std::set<TimeIntegrator *>
PIMPLE::getTimeIntegrators() const
{
  // We use a set because time integrators were added to every system, and we want a unique
  std::set<TimeIntegrator *> tis;
  // Get all time integrators from the systems in the FEProblemSolve
  for (const auto sys : pimpleSolve().systemsToSolve())
    for (const auto & ti : sys->getTimeIntegrators())
      tis.insert(ti.get());
  return tis;
}
