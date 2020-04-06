//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExternalPetscTimeStepper.h"

registerMooseObject("ExternalPetscSolverApp", ExternalPetscTimeStepper);

InputParameters
ExternalPetscTimeStepper::validParams()
{
  InputParameters params = TimeStepper::validParams();

  params.addClassDescription("Timestepper that queries the step size of the external petsc solver, "
                             "and use that as the time step");
  return params;
}

ExternalPetscTimeStepper::ExternalPetscTimeStepper(const InputParameters & parameters)
  : TimeStepper(parameters),
    // ExternalPetscTimeStepper always requires ExternalPetscSolverApp
    _petsc_app(static_cast<ExternalPetscSolverApp &>(_app))
#if LIBMESH_HAVE_PETSC
    ,
    _ts(_petsc_app.getExternalPETScTS())
#endif
{
}

Real
ExternalPetscTimeStepper::computeInitialDT()
{
#if LIBMESH_HAVE_PETSC
  PetscReal dt;
  TSGetTimeStep(_ts, &dt);
  return dt;
#endif
}

Real
ExternalPetscTimeStepper::computeDT()
{
#if LIBMESH_HAVE_PETSC
  PetscReal dt;
  TSGetTimeStep(_ts, &dt);
  return dt;
#endif
}

void
ExternalPetscTimeStepper::preSolve()
{
#if LIBMESH_HAVE_PETSC
  // Old solution is the initial condition of this time step
  VecCopy(_petsc_app.getExternalPETScTSSolutionOld(), _petsc_app.getExternalPETScTSSolution());
#endif
}
