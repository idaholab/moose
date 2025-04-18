//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StaticCondensationFieldSplitPreconditioner.h"

// MOOSE includes
#include "PetscSupport.h"
#include "NonlinearSystemBase.h"
#include "PetscDMMoose.h"
#include "Split.h"

#include "libmesh/static_condensation.h"
#include "libmesh/petsc_linear_solver.h"

using namespace libMesh;

registerMooseObjectAliased("MooseApp", StaticCondensationFieldSplitPreconditioner, "SCFSP");

InputParameters
StaticCondensationFieldSplitPreconditioner::validParams()
{
  return FieldSplitPreconditionerTempl<MooseStaticCondensationPreconditioner>::validParams();
}

StaticCondensationFieldSplitPreconditioner::StaticCondensationFieldSplitPreconditioner(
    const InputParameters & params)
  : FieldSplitPreconditionerTempl<MooseStaticCondensationPreconditioner>(params)
{
  std::shared_ptr<Split> top_split = _nl.getSplit(_decomposition_split);
  top_split->setup(_nl, prefix());
}

const libMesh::DofMapBase &
StaticCondensationFieldSplitPreconditioner::dofMap() const
{
  return sc();
}

const libMesh::System &
StaticCondensationFieldSplitPreconditioner::system() const
{
  return sc().reduced_system();
}

std::string
StaticCondensationFieldSplitPreconditioner::petscPrefix() const
{
  return MooseStaticCondensationPreconditioner::petscPrefix();
}

void
StaticCondensationFieldSplitPreconditioner::setupDM()
{
  PetscBool ismoose;
  DM dm = LIBMESH_PETSC_NULLPTR;

  // Initialize the part of the DM package that's packaged with Moose; in the PETSc source tree this
  // call would be in DMInitializePackage()
  LibmeshPetscCallA(_nl.comm().get(), DMMooseRegisterAll());
  // Create and set up the DM that will consume the split options and deal with block matrices.
  auto & petsc_solver = cast_ref<PetscLinearSolver<Number> &>(sc().reduced_system_solver());
  auto ksp = petsc_solver.ksp();
  // if there exists a DMMoose object, not to recreate a new one
  LibmeshPetscCallA(_nl.comm().get(), KSPGetDM(ksp, &dm));
  if (dm)
  {
    LibmeshPetscCallA(_nl.comm().get(), PetscObjectTypeCompare((PetscObject)dm, DMMOOSE, &ismoose));
    if (ismoose)
      return;
  }
  createMooseDM(&dm);
  LibmeshPetscCallA(_nl.comm().get(), KSPSetDM(ksp, dm));
  // We set the operators ourselves. We do not want the DM to generate the operators
  LibmeshPetscCallA(_nl.comm().get(), KSPSetDMActive(ksp, PETSC_FALSE));
  LibmeshPetscCallA(_nl.comm().get(), DMDestroy(&dm));
}
