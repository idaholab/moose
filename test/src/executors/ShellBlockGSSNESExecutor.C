//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShellBlockGSSNESExecutor.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

#include "libmesh/petsc_solver_exception.h"
#include "libmesh/petsc_vector.h"
#include <petscsnes.h>

registerMooseObject("MooseTestApp", ShellBlockGSSNESExecutor);

InputParameters
ShellBlockGSSNESExecutor::validParams()
{
  InputParameters params = SNESExecutor::validParams();
  params.addClassDescription(
      "SNESSHELL-based block Gauss-Seidel executor for testing the multi-system Case 3 "
      "path of NewtonSNESExecutor.  Sweeps the listed nonlinear systems in order, calling "
      "_fe_problem.solve() for each.");
  params.addRequiredParam<std::vector<ExecutorName>>(
      "sub_snes_executors", "The sub-SNES executors who we will sweep over");
  MooseEnum sweep_type("multiplicative symmetric_multiplicative", "multiplicative");
  params.addParam<MooseEnum>("sweep_type", sweep_type, "multiplicative performs a forward sweep "
                             "only; symmetric_multiplicative adds a backward sweep (1..N, N-1..1)");
  return params;
}

ShellBlockGSSNESExecutor::ShellBlockGSSNESExecutor(const InputParameters & params)
  : SNESExecutor(params), _sweep_type(getParam<MooseEnum>("sweep_type"))
{
  for (const auto & name : getParam<std::vector<ExecutorName>>("sub_snes_executors"))
    _sub_snes.push_back(&getExecutorByName<SNESExecutor>(name));
}

void
ShellBlockGSSNESExecutor::setupSNES()
{
  LibmeshPetscCallA(this->comm().get(), SNESCreate(this->comm().get(), &_snes));
  LibmeshPetscCallA(this->comm().get(), SNESSetType(_snes, SNESSHELL));
  LibmeshPetscCallA(this->comm().get(), SNESSetApplicationContext(_snes, this));
  LibmeshPetscCallA(this->comm().get(), SNESShellSetSolve(_snes, shellSolveCallback));
  _snes_setup_done = true;
}

Executor::Result
ShellBlockGSSNESExecutor::run()
{
  auto & result = newResult();

  for (auto * const sub : _sub_snes)
    result.record(sub->name(), sub->exec());

  // Backward sweep for symmetric multiplicative, excluding the last element of the
  // forward sweep (which was just solved and has not had any inputs change since).
  if (_sweep_type == "symmetric_multiplicative")
    for (auto it = std::next(_sub_snes.rbegin()); it != _sub_snes.rend(); ++it)
      result.record((*it)->name(), (*it)->exec());

  return result;
}

// Will be called when this is a preconditioner
PetscErrorCode
ShellBlockGSSNESExecutor::shellSolveCallback(SNES snes, Vec x)
{
  PetscFunctionBegin;
  void * ctx;
  PetscCall(SNESGetApplicationContext(snes, &ctx));
  auto * const ex = static_cast<ShellBlockGSSNESExecutor *>(ctx);

  // x is a VecNest holding the current Newton iterate (copied there by SNESApplyNPC).
  // Copy it into the libmesh solution vecs so sub-solves start from the right point.
  PetscInt n_sub;
  PetscCall(VecNestGetSize(x, &n_sub));
  for (PetscInt i = 0; i < n_sub; ++i)
  {
    Vec sub_x;
    PetscCall(VecNestGetSubVec(x, i, &sub_x));
    Vec sol_i =
        cast_ptr<libMesh::PetscVector<libMesh::Number> *>(
            ex->_fe_problem.getNonlinearSystem(i).system().solution.get())
            ->vec();
    PetscCall(VecCopy(sub_x, sol_i));
  }

  ex->run();

  // Write the NPC output (updated libmesh solution vecs) back into x so SNESApplyNPC
  // can compute the fixed-point residual x - NPC(x).
  for (PetscInt i = 0; i < n_sub; ++i)
  {
    Vec sub_x;
    PetscCall(VecNestGetSubVec(x, i, &sub_x));
    Vec sol_i =
        cast_ptr<libMesh::PetscVector<libMesh::Number> *>(
            ex->_fe_problem.getNonlinearSystem(i).system().solution.get())
            ->vec();
    PetscCall(VecCopy(sol_i, sub_x));
  }

  PetscFunctionReturn(PETSC_SUCCESS);
}
