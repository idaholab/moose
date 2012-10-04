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

#ifndef PETSCSUPPORT_H
#define PETSCSUPPORT_H

#include "libmesh.h"

#ifdef LIBMESH_HAVE_PETSC

#include "Problem.h"
// libMesh
#include "petsc_nonlinear_solver.h"

class FEProblem;

namespace Moose
{
namespace PetscSupport
{
void petscSetOptions(Problem & problem);

void petscSetDefaults(FEProblem & problem);

void petscSetupDampers(NonlinearImplicitSystem& sys);

}
}

#endif //LIBMESH_HAVE_PETSC

#endif //PETSCSUPPORT_H
