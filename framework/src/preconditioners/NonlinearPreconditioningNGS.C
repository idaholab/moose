//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlinearPreconditioningNGS.h"
#include "FEProblem.h"

#include "libmesh/petsc_solver_exception.h"
#include <petscsnes.h>

registerMooseObjectAliased("MooseApp", NonlinearPreconditioningNGS, "NGS");

InputParameters
NonlinearPreconditioningNGS::validParams()
{
  InputParameters params = NonlinearPreconditioning::validParams();
  params.addClassDescription(
      "Nonlinear Gauss-Seidel (NGS) preconditioning using PETSc SNESNGS. "
      "Each NPC application sweeps through the listed inner systems in order.");
  return params;
}

NonlinearPreconditioningNGS::NonlinearPreconditioningNGS(const InputParameters & params)
  : NonlinearPreconditioning(params)
{
}

void
NonlinearPreconditioningNGS::setupNPC()
{
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESCreate(_fe_problem.comm().get(), &_npc_snes));
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESSetType(_npc_snes, SNESNGS));
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESSetNGS(_npc_snes, ngsCallback, this));
}

PetscErrorCode
NonlinearPreconditioningNGS::ngsCallback(SNES /*snes*/, Vec /*x*/, Vec /*b*/, void * ctx)
{
  PetscFunctionBegin;
  auto * npc = static_cast<NonlinearPreconditioningNGS *>(ctx);
  for (const auto sys_num : npc->_inner_sys_nums)
    npc->_fe_problem.solve(sys_num);
  PetscFunctionReturn(PETSC_SUCCESS);
}
