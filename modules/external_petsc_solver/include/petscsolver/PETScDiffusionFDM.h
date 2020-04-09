//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh.h"

#include "libmesh/petsc_macro.h"
#include <petscdm.h>
#include <petscdmda.h>
#include <petscts.h>
#include <petsc/private/petscimpl.h>

PETSC_EXTERN PetscErrorCode
externalPETScDiffusionFDMSolve(TS, Vec, Vec, PetscReal, PetscReal, PetscBool *);
PETSC_EXTERN PetscErrorCode PETScExternalSolverCreate(MPI_Comm, TS *);
PETSC_EXTERN PetscErrorCode PETScExternalSolverDestroy(TS);
PETSC_EXTERN PetscErrorCode FormIFunction(TS, PetscReal, Vec, Vec, Vec, void *);
PETSC_EXTERN PetscErrorCode FormIJacobian(TS, PetscReal, Vec, Vec, PetscReal, Mat, Mat, void *);
PETSC_EXTERN PetscErrorCode FormInitialSolution(TS, Vec, void *);
