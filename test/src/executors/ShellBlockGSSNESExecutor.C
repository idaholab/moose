//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShellBlockGSSNESExecutor.h"
#include "NewtonSNESExecutor.h"

#include "libmesh/petsc_solver_exception.h"
#include "libmesh/petsc_vector.h"
#include <petscsnes.h>

#include <cstddef>
#include <iterator>

registerMooseObject("MooseTestApp", ShellBlockGSSNESExecutor);

InputParameters
ShellBlockGSSNESExecutor::validParams()
{
  InputParameters params = SNESNPCExecutor::validParams();
  params.addRequiredParam<std::vector<ExecutorName>>(
      "sub_snes_executors", "The sub-SNES executors who we will sweep over");
  MooseEnum sweep_type("multiplicative symmetric_multiplicative", "multiplicative");
  params.addParam<MooseEnum>("sweep_type",
                             sweep_type,
                             "multiplicative performs a forward sweep "
                             "only; symmetric_multiplicative adds a backward sweep (1..N, N-1..1)");
  return params;
}

ShellBlockGSSNESExecutor::ShellBlockGSSNESExecutor(const InputParameters & params)
  : SNESNPCExecutor(params), _sweep_type(getParam<MooseEnum>("sweep_type"))
{
  for (const auto & name : getParam<std::vector<ExecutorName>>("sub_snes_executors"))
    _sub_snes.push_back(&getExecutorByName<NewtonSNESExecutor>(name));
}

ShellBlockGSSNESExecutor::~ShellBlockGSSNESExecutor()
{
  if (_block_residual)
    PetscCallAbort(this->comm().get(), VecDestroy(&_block_residual));
  if (_block_update)
    PetscCallAbort(this->comm().get(), VecDestroy(&_block_update));
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
    auto & lm_sys = ex->_sub_snes[i]->getSystem();
    Vec sol_i = cast_ptr<libMesh::PetscVector<libMesh::Number> *>(lm_sys.solution.get())->vec();
    PetscCall(VecCopy(sub_x, sol_i));
    // Now we must update the current local solution
    lm_sys.update();
  }

  ex->run();

  // Write the NPC output (updated libmesh solution vecs) back into x so SNESApplyNPC
  // can compute the fixed-point residual x - NPC(x).
  for (PetscInt i = 0; i < n_sub; ++i)
  {
    Vec sub_x;
    PetscCall(VecNestGetSubVec(x, i, &sub_x));
    auto & lm_sys = ex->_sub_snes[i]->getSystem();
    Vec sol_i = cast_ptr<libMesh::PetscVector<libMesh::Number> *>(lm_sys.solution.get())->vec();
    PetscCall(VecCopy(sol_i, sub_x));
  }

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
ShellBlockGSSNESExecutor::applyBA(Mat A, Vec X, Vec Y)
{
  PetscFunctionBegin;
  if (!_work)
    PetscCall(VecDuplicate(X, &_work));
  if (!_block_residual)
    PetscCall(VecDuplicate(X, &_block_residual));
  if (!_block_update)
    PetscCall(VecDuplicate(X, &_block_update));

  PetscCall(MatMult(A, X, _work));
  PetscCall(VecSet(Y, 0.0));

  for (const auto i : index_range(_sub_snes))
    PetscCall(applyBlockUpdate(A, _work, Y, static_cast<PetscInt>(i)));

  if (_sweep_type == "symmetric_multiplicative")
    for (auto it = std::next(_sub_snes.rbegin()); it != _sub_snes.rend(); ++it)
    {
      const auto i = static_cast<PetscInt>(std::distance(it, _sub_snes.rend()) - 1);
      PetscCall(applyBlockUpdate(A, _work, Y, i));
    }

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
ShellBlockGSSNESExecutor::applyBlockUpdate(Mat A, Vec rhs, Vec Y, PetscInt i)
{
  KSP sub_ksp;
  Vec rhs_i, residual_i, update_i, Y_i;

  PetscFunctionBegin;
  PetscCall(VecNestGetSubVec(rhs, i, &rhs_i));
  PetscCall(VecNestGetSubVec(_block_residual, i, &residual_i));
  PetscCall(VecNestGetSubVec(_block_update, i, &update_i));
  PetscCall(VecNestGetSubVec(Y, i, &Y_i));

  PetscCall(VecCopy(rhs_i, residual_i));
  PetscCall(VecZeroEntries(update_i));
  for (const auto j : index_range(_sub_snes))
  {
    Mat A_ij;
    Vec Y_j;
    PetscCall(MatNestGetSubMat(A, i, static_cast<PetscInt>(j), &A_ij));
    if (!A_ij)
      continue;

    PetscCall(VecNestGetSubVec(Y, static_cast<PetscInt>(j), &Y_j));
    PetscCall(MatMultAdd(A_ij, Y_j, update_i, update_i));
  }
  PetscCall(VecAXPY(residual_i, -1.0, update_i));
  PetscCall(VecZeroEntries(update_i));

  auto sub_snes = _sub_snes[static_cast<std::size_t>(i)]->getSNES();
  PetscCall(SNESGetKSP(sub_snes, &sub_ksp));
  PetscCall(KSPSolve(sub_ksp, residual_i, update_i));
  PetscCall(VecAXPY(Y_i, 1.0, update_i));

  PetscFunctionReturn(PETSC_SUCCESS);
}
