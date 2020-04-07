//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExternalPetscTimeStepper.h"
#include "ExternalPETScProblem.h"

registerMooseObject("ExternalPetscSolverApp", ExternalPetscTimeStepper);

InputParameters
ExternalPetscTimeStepper::validParams()
{
  InputParameters params = TimeStepper::validParams();

  params.addClassDescription("Timestepper that queries the step size of the external petsc solver, "
                             "and use that as the time step size.");
  return params;
}

ExternalPetscTimeStepper::ExternalPetscTimeStepper(const InputParameters & parameters)
  : TimeStepper(parameters),
    // ExternalPetscTimeStepper always requires ExternalPETScProblem
    _external_petsc_problem(static_cast<ExternalPETScProblem &>(_fe_problem))
{
}

Real
ExternalPetscTimeStepper::computeInitialDT()
{
  // Query the time step size of PETSc solver
  PetscReal dt;
  TSGetTimeStep(_external_petsc_problem.getPetscTS(), &dt);
  return dt;
}

Real
ExternalPetscTimeStepper::computeDT()
{
  // Query the time step size of PETSc solver
  PetscReal dt;
  TSGetTimeStep(_external_petsc_problem.getPetscTS(), &dt);
  return dt;
}
