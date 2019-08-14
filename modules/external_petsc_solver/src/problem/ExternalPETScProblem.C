//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExternalPETScProblem.h"
#include "SystemBase.h"

registerMooseObject("ExternalPetscSolverApp", ExternalPETScProblem);

template <>
InputParameters
validParams<ExternalPETScProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  params.addRequiredParam<VariableName>("sync_variable",
                                        "The variable PETSc external solution will be synced to");
  return params;
}

ExternalPETScProblem::ExternalPETScProblem(const InputParameters & params)
  : ExternalProblem(params),
    _sync_to_var_name(getParam<VariableName>("sync_variable")),
    // ExternalPETScProblem always requires ExternalPetscSolverApp
    _petsc_app(static_cast<ExternalPetscSolverApp &>(_app))
#if LIBMESH_HAVE_PETSC
    ,
    _ts(_petsc_app.getExternalPETScTS())
{
  DM da;
  TSGetDM(_ts, &da);
  DMCreateGlobalVector(da, &_petsc_sol);
  FormInitialSolution(_ts, _petsc_sol, NULL);
}
#else
{
  mooseError("You need to have PETSc installed to use ExternalPETScProblem");
}
#endif

void
ExternalPETScProblem::externalSolve()
{
#if LIBMESH_HAVE_PETSC
  _console << "PETSc External Solve!" << std::endl;
  externalPETScDiffusionFDMSolve(_ts, _petsc_sol, dt(), time());
#endif
}

void
ExternalPETScProblem::syncSolutions(Direction direction)
{
#if LIBMESH_HAVE_PETSC
  if (direction == Direction::FROM_EXTERNAL_APP)
  {
    _console << "syncSolutions from external petsc App" << std::endl;
    DM da;
    // xs: start grid point in x direction on local
    // ys: start grid point in y direciton on local
    // xm: number of grid points in x direciton on local
    // ym: number of grid points in y direction on local
    // Mx: number of grid points in x direction on all processors
    PetscInt i, j, xs, ys, xm, ym, Mx;
    PetscScalar ** _petsc_sol_array;
    TSGetDM(_ts, &da);
    DMDAGetInfo(da,
                PETSC_IGNORE,
                &Mx,
                PETSC_IGNORE,
                PETSC_IGNORE,
                PETSC_IGNORE,
                PETSC_IGNORE,
                PETSC_IGNORE,
                PETSC_IGNORE,
                PETSC_IGNORE,
                PETSC_IGNORE,
                PETSC_IGNORE,
                PETSC_IGNORE,
                PETSC_IGNORE);
    DMDAGetCorners(da, &xs, &ys, NULL, &xm, &ym, NULL);
    DMDAVecGetArray(da, _petsc_sol, &_petsc_sol_array);

    // Take the solution from PETSc, and sync it to one MOOSE variable
    // We currently support one variable only but it is straightforward
    // to have multiple moose variables
    MeshBase & to_mesh = mesh().getMesh();
    auto & sync_to_var = getVariable(
        0, _sync_to_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);

    for (j = ys; j < ys + ym; j++)
      for (i = xs; i < xs + xm; i++)
      {
        Node * to_node = to_mesh.node_ptr(i + j * Mx);
        if (to_node->n_comp(sync_to_var.sys().number(), sync_to_var.number()) > 1)
          mooseError("Does not support multiple components");

        dof_id_type dof = to_node->dof_number(sync_to_var.sys().number(), sync_to_var.number(), 0);
        // Copy the solution to the right location
        sync_to_var.sys().solution().set(dof, _petsc_sol_array[j][i]);
      }

    sync_to_var.sys().solution().close();

    DMDAVecRestoreArray(da, _petsc_sol, &_petsc_sol_array);
  }
  else if (direction == Direction::TO_EXTERNAL_APP)
  {
    _console << "syncSolutions to external petsc App " << std::endl;
    // We could the similar thing to sync the solution back to PETSc.
  }
#endif
}
