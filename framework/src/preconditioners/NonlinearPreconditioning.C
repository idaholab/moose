//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlinearPreconditioning.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

#include "libmesh/petsc_solver_exception.h"

registerMooseObject("MooseApp", NonlinearPreconditioning);

InputParameters
NonlinearPreconditioning::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addClassDescription(
      "Nonlinear preconditioning for multi-system solves: each outer Newton step is preceded by "
      "inner SNES solves for the listed systems (nonlinear elimination when only a subset is "
      "listed, nonlinear Gauss-Seidel when all systems are listed).");
  params.addRequiredParam<std::vector<NonlinearSystemName>>(
      "inner_nl_sys_names",
      "Names of nonlinear systems to solve as the inner nonlinear preconditioner before each "
      "outer Newton step.");
  return params;
}

NonlinearPreconditioning::NonlinearPreconditioning(const InputParameters & params)
  : MooseObject(params),
    PerfGraphInterface(this),
    _fe_problem(*params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
  for (const auto & name : getParam<std::vector<NonlinearSystemName>>("inner_nl_sys_names"))
    _inner_sys_nums.push_back(_fe_problem.nlSysNum(name));
}

NonlinearPreconditioning::~NonlinearPreconditioning()
{
#ifdef LIBMESH_HAVE_PETSC
  if (_npc_snes)
    SNESDestroy(&_npc_snes);
#endif
}

void
NonlinearPreconditioning::initialSetup()
{
#ifdef LIBMESH_HAVE_PETSC
  setupNPC();
  // Per-solve NPC wiring is done in wireToSNES(), called from FEProblemBase::solve() before each
  // outer solve.  This is necessary because libMesh destroys and recreates the SNES after every
  // solve(), so SNESSetNPC must be called again on the fresh SNES each time.
#endif
}

void
NonlinearPreconditioning::wireToSNES(unsigned int sys_num)
{
#ifdef LIBMESH_HAVE_PETSC
  SNES outer_snes = _fe_problem.getNonlinearSystem(sys_num).getSNES();
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESSetNPC(outer_snes, _npc_snes));
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESSetNPCSide(outer_snes, PC_LEFT));
#endif
}

#ifdef LIBMESH_HAVE_PETSC
void
NonlinearPreconditioning::setupNPC()
{
  LibmeshPetscCallA(_fe_problem.comm().get(),
                    SNESCreate(_fe_problem.comm().get(), &_npc_snes));
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESSetType(_npc_snes, SNESSHELL));
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESShellSetSolve(_npc_snes, npcShellSolve));
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESSetApplicationContext(_npc_snes, this));
}

PetscErrorCode
NonlinearPreconditioning::npcShellSolve(SNES snes, Vec /*x*/)
{
  PetscFunctionBegin;
  void * ctx;
  PetscCall(SNESGetApplicationContext(snes, &ctx));
  auto * npc = static_cast<NonlinearPreconditioning *>(ctx);
  for (const auto sys_num : npc->_inner_sys_nums)
    npc->_fe_problem.solve(sys_num);
  PetscFunctionReturn(PETSC_SUCCESS);
}
#endif
