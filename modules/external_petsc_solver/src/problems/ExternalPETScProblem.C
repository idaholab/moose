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
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_solver_exception.h"

registerMooseObject("ExternalPetscSolverApp", ExternalPETScProblem);

InputParameters
ExternalPETScProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addRequiredParam<VariableName>("sync_variable",
                                        "The variable PETSc external solution will be synced to");
  return params;
}

ExternalPETScProblem::ExternalPETScProblem(const InputParameters & params)
  : ExternalProblem(params),
    _sync_to_var_name(getParam<VariableName>("sync_variable")),
    // Require ExternalPetscSolverApp
    _external_petsc_app(static_cast<ExternalPetscSolverApp &>(_app)),
    _ts(_external_petsc_app.getPetscTS()),
    // RestartableData is required for recovering when PETSc solver runs as a master app
    _petsc_sol(declareRestartableData<Vec>("petsc_sol")),
    _petsc_sol_old(declareRestartableData<Vec>("petsc_sol_old")),
    _petsc_udot(declareRestartableData<Vec>("petsc_udot"))
{
  DM da;
  auto ierr = TSGetDM(_ts, &da);
  LIBMESH_CHKERR(ierr);
  // Create a global solution vector
  ierr = DMCreateGlobalVector(da, &_petsc_sol);
  LIBMESH_CHKERR(ierr);
  // This is required because libMesh incorrectly treats the PETSc parallel vector as a ghost vector
  // We should be able to remove this line of code once libMesh is updated
  ierr = VecMPISetGhost(_petsc_sol, 0, nullptr);
  LIBMESH_CHKERR(ierr);
  // The solution at the previous time step
  ierr = VecDuplicate(_petsc_sol, &_petsc_sol_old);
  LIBMESH_CHKERR(ierr);
  // Udot
  ierr = VecDuplicate(_petsc_sol, &_petsc_udot);
  LIBMESH_CHKERR(ierr);
  // RHS
  ierr = VecDuplicate(_petsc_sol, &_petsc_rhs);
  LIBMESH_CHKERR(ierr);
  // Form an initial condition
  ierr = FormInitialSolution(_ts, _petsc_sol, NULL);
  LIBMESH_CHKERR(ierr);
  ierr = VecCopy(_petsc_sol, _petsc_sol_old);
  LIBMESH_CHKERR(ierr);
  ierr = VecSet(_petsc_udot, 0);
  LIBMESH_CHKERR(ierr);
}

ExternalPETScProblem::~ExternalPETScProblem()
{
  // Destroy all handles of external Petsc solver
  auto ierr = VecDestroy(&_petsc_sol);
  ierr = VecDestroy(&_petsc_sol_old);
  ierr = VecDestroy(&_petsc_udot);
  ierr = VecDestroy(&_petsc_rhs);
  // Don't throw during destruction, just abort
  CHKERRABORT(this->comm().get(), ierr);
}

void
ExternalPETScProblem::externalSolve()
{
  _console << "PETSc External Solve!" << std::endl;
  // "_petsc_sol_old" is the solution of the current time step, and "_petsc_sol" will be updated
  // to store the solution of the next time step after this call.
  // This call advances a time step so that there is an opportunity to
  // exchange information with MOOSE simulations.
  auto ierr = externalPETScDiffusionFDMSolve(
      _ts, _petsc_sol_old, _petsc_sol, dt(), time(), &_petsc_converged);
  LIBMESH_CHKERR(ierr);
}

// This function is called when MOOSE time stepper actually moves to the next time step
// "PostTimeStep" may not be called for certain cases (e.g., auto_advance=false)
void
ExternalPETScProblem::advanceState()
{
  FEProblemBase::advanceState();
  // Compute udot using a backward Euler method
  // If the external code uses a different method,
  // udot should be retrieved from the external solver
  auto ierr = VecCopy(_petsc_sol, _petsc_udot);
  LIBMESH_CHKERR(ierr);
  ierr = VecAXPY(_petsc_udot, -1., _petsc_sol_old);
  LIBMESH_CHKERR(ierr);
  ierr = VecScale(_petsc_udot, 1. / dt());
  LIBMESH_CHKERR(ierr);
  // Save current solution because we are moving to the next time step
  ierr = VecCopy(_petsc_sol, _petsc_sol_old);
  LIBMESH_CHKERR(ierr);
}

Real
ExternalPETScProblem::computeResidualL2Norm()
{
  auto ierr = TSComputeIFunction(_ts, time(), _petsc_sol, _petsc_udot, _petsc_rhs, PETSC_FALSE);
  LIBMESH_CHKERR(ierr);
  PetscReal norm;
  ierr = VecNorm(_petsc_rhs, NORM_2, &norm);
  LIBMESH_CHKERR(ierr);

  return norm;
}

void
ExternalPETScProblem::syncSolutions(Direction direction)
{
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
    auto ierr = TSGetDM(_ts, &da);
    LIBMESH_CHKERR(ierr);
    ierr = DMDAGetInfo(da,
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
    LIBMESH_CHKERR(ierr);
    ierr = DMDAGetCorners(da, &xs, &ys, NULL, &xm, &ym, NULL);
    LIBMESH_CHKERR(ierr);
    ierr = DMDAVecGetArray(da, _petsc_sol, &_petsc_sol_array);
    LIBMESH_CHKERR(ierr);

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
        // For the current example, we need to update only one variable.
        // This line of code is used to make sure users won't make a mistake in the demo input file.
        // If multiple variables need to be transfered for some use cases, users should
        // loop over variables and copy necessary data.
        if (to_node->n_comp(sync_to_var.sys().number(), sync_to_var.number()) > 1)
          mooseError("Does not support multiple components");

        dof_id_type dof = to_node->dof_number(sync_to_var.sys().number(), sync_to_var.number(), 0);
        // Copy the solution to the right location
        sync_to_var.sys().solution().set(dof, _petsc_sol_array[j][i]);
      }

    sync_to_var.sys().solution().close();

    ierr = DMDAVecRestoreArray(da, _petsc_sol, &_petsc_sol_array);
    LIBMESH_CHKERR(ierr);

    // Make the solution and the current local solution consistent
    sync_to_var.sys().update();
  }
  else if (direction == Direction::TO_EXTERNAL_APP)
  {
    _console << "syncSolutions to external petsc App " << std::endl;
    // We could the similar thing to sync the solution back to PETSc.
  }
}
