//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NewtonSNESExecutor.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "NonlinearSystemBase.h"

#include "libmesh/libmesh.h"
#include "libmesh/petsc_solver_exception.h"
#include "libmesh/implicit_system.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"

#include <petscsnes.h>
#include <petscsys.h>

registerMooseObject("MooseApp", NewtonSNESExecutor);

InputParameters
NewtonSNESExecutor::validParams()
{
  InputParameters params = SNESExecutor::validParams();
  params.addClassDescription(
      "Newton-type outer solver executor (SNESEWTONLS). "
      "If 'nonlinear_system_name' is supplied, solves only that system via _fe_problem.solve(). "
      "If omitted with a single NL system, solves system 0. "
      "If omitted with multiple NL systems, builds a combined outer SNES with VecNest/MatNest; "
      "nl_preconditioning is required in that path.");
  params.addParam<NonlinearSystemName>(
      "nonlinear_system_name",
      "Name of a specific nonlinear system to solve (Cases 1/2). "
      "Omit to use the multi-system combined path (Case 3).");
  return params;
}

NewtonSNESExecutor::NewtonSNESExecutor(const InputParameters & params) : SNESExecutor(params)
{
  if (isParamSetByUser("nonlinear_system_name"))
    _fixed_sys_num =
        (int)_fe_problem.nlSysNum(getParam<NonlinearSystemName>("nonlinear_system_name"));
}

NewtonSNESExecutor::~NewtonSNESExecutor()
{
  if (_vec_sol)
    PetscCallAbort(this->comm().get(), VecDestroy(&_vec_sol));
  if (_mat_nest)
    PetscCallAbort(this->comm().get(), MatDestroy(&_mat_nest));
  // _snes and _vec_func are destroyed by ~SNESExecutor
}

void
NewtonSNESExecutor::setupSNES()
{
  // setupSNES() is only entered on the Case 3 (multi-system) path.
  const unsigned int n_sys = _fe_problem.numNonlinearSystems();

  // Build VecNest for solution and (if outer solver) residual.
  std::vector<Vec> sol_vecs(n_sys);
  std::vector<Vec> rhs_vecs(n_sys);
  for (unsigned int i = 0; i < n_sys; ++i)
  {
    auto & sys_i = _fe_problem.getNonlinearSystem(i);
    sol_vecs[i] =
        cast_ptr<libMesh::PetscVector<libMesh::Number> *>(sys_i.system().solution.get())->vec();
    rhs_vecs[i] = cast_ptr<libMesh::PetscVector<libMesh::Number> *>(&sys_i.RHS())->vec();
  }
  LibmeshPetscCallA(this->comm().get(),
                    VecCreateNest(this->comm().get(), n_sys, nullptr, sol_vecs.data(), &_vec_sol));
  if (!_is_npc)
    LibmeshPetscCallA(
        this->comm().get(),
        VecCreateNest(this->comm().get(), n_sys, nullptr, rhs_vecs.data(), &_vec_func));

  allocateOffDiagMats();
  buildMatNest();

  LibmeshPetscCallA(this->comm().get(), SNESCreate(this->comm().get(), &_snes));

  if (!_is_npc)
  {
    LibmeshPetscCallA(
        this->comm().get(),
        SNESSetFunction(_snes, _vec_func, outerResidualCallback, this));
    LibmeshPetscCallA(
        this->comm().get(),
        SNESSetJacobian(_snes, _mat_nest, _mat_nest, outerJacobianCallback, this));
    registerFieldSplitIS(_snes, n_sys);
    if (_npc_executor)
    {
      LibmeshPetscCallA(this->comm().get(), SNESSetNPC(_snes, _npc_executor->getSNES()));
      LibmeshPetscCallA(this->comm().get(), SNESSetNPCSide(_snes, PC_LEFT));
    }
  }

  _snes_setup_done = true;
}

Executor::Result
NewtonSNESExecutor::run()
{
  const unsigned int n_sys = _fe_problem.numNonlinearSystems();

  // Determine which path to take.
  if (!_multi_system && _fixed_sys_num < 0)
  {
    if (n_sys == 1)
      _fixed_sys_num = 0;
    else
      _multi_system = true;
  }

  if (_multi_system)
  {
    // Case 3: combined outer Newton with VecNest/MatNest.
    if (!_npc_executor)
      mooseError(
          "NewtonSNESExecutor: multiple nonlinear systems require 'nl_preconditioning' to be set.");

    if (!_snes_setup_done)
      setupSNES();
    else
      buildMatNest();

    LibmeshPetscCallA(this->comm().get(), SNESSolve(_snes, nullptr, _vec_sol));

    SNESConvergedReason reason;
    LibmeshPetscCallA(this->comm().get(), SNESGetConvergedReason(_snes, &reason));
    auto & result = newResult();
    if (reason > 0)
      result.pass("SNES converged");
    else
      result.fail("SNES diverged");
    return result;
  }

  // Cases 1 and 2: delegate to _fe_problem.solve(sys_num).
  const unsigned int sys_num = (unsigned int)_fixed_sys_num;

  if (_npc_executor)
  {
    // Wire the NPC onto the system's freshly-created SNES each solve via the hook.
    _fe_problem.setNLPreSolveHook([this, sys_num](unsigned int hook_sys_num)
    {
      if (hook_sys_num != sys_num)
        return;
      SNES outer = _fe_problem.getNonlinearSystem(sys_num).getSNES();
      LibmeshPetscCallA(this->comm().get(), SNESSetNPC(outer, _npc_executor->getSNES()));
      LibmeshPetscCallA(this->comm().get(), SNESSetNPCSide(outer, PC_LEFT));
    });
  }

  _fe_problem.solve(sys_num);

  if (_npc_executor)
    _fe_problem.setNLPreSolveHook({});

  auto & result = newResult();
  if (_fe_problem.solverSystemConverged(sys_num))
    result.pass("solver converged");
  else
    result.fail("solver diverged");
  return result;
}

void
NewtonSNESExecutor::allocateOffDiagMats()
{
  const unsigned int n_sys = _fe_problem.numNonlinearSystems();
  for (unsigned int i = 0; i < n_sys; ++i)
  {
    auto & sys_i = _fe_problem.getNonlinearSystemBase(i);
    for (unsigned int j = 0; j < n_sys; ++j)
    {
      if (i == j)
        continue;

      auto & sys_j = _fe_problem.getNonlinearSystemBase(j);

      const auto m = sys_i.system().n_dofs();
      const auto n = sys_j.system().n_dofs();
      const auto m_l = sys_i.system().n_local_dofs();
      const auto n_l = sys_j.system().n_local_dofs();

      auto mat = std::make_unique<libMesh::PetscMatrix<libMesh::Number>>(_fe_problem.comm());
      mat->init(m, n, m_l, n_l, 30, 10);
      LibmeshPetscCallA(_fe_problem.comm().get(),
                        MatSetOption(mat->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE));

      const TagID tag =
          _fe_problem.addMatrixTag("NPC_J_" + std::to_string(i) + "_" + std::to_string(j));
      sys_i.associateMatrixToTag(*mat, tag);

      _off_diag_tags[{i, j}] = tag;
      _off_diag_mats[{i, j}] = std::move(mat);
    }
  }
}

void
NewtonSNESExecutor::buildMatNest()
{
  const unsigned int n_sys = _fe_problem.numNonlinearSystems();

  if (_mat_nest)
  {
    LibmeshPetscCallA(_fe_problem.comm().get(), MatDestroy(&_mat_nest));
    _mat_nest = nullptr;
  }

  std::vector<Mat> sub_mats(n_sys * n_sys, nullptr);
  for (unsigned int i = 0; i < n_sys; ++i)
  {
    auto & sys_i = _fe_problem.getNonlinearSystemBase(i);
    auto & J_ii = static_cast<libMesh::PetscMatrix<libMesh::Number> &>(
        static_cast<libMesh::ImplicitSystem &>(sys_i.system()).get_system_matrix());
    sub_mats[i * n_sys + i] = J_ii.mat();

    for (unsigned int j = 0; j < n_sys; ++j)
    {
      if (i == j)
        continue;
      const std::pair<unsigned int, unsigned int> key{i, j};
      sub_mats[i * n_sys + j] = libmesh_map_find(_off_diag_mats, key)->mat();
    }
  }

  LibmeshPetscCallA(
      _fe_problem.comm().get(),
      MatCreateNest(
          _fe_problem.comm().get(), n_sys, nullptr, n_sys, nullptr, sub_mats.data(), &_mat_nest));
}

void
NewtonSNESExecutor::assembleOffDiagJacobian()
{
  const unsigned int n_sys = _fe_problem.numNonlinearSystems();
  for (unsigned int i = 0; i < n_sys; ++i)
  {
    auto & sys_i = _fe_problem.getNonlinearSystemBase(i);
    const TagID sys_tag_i = sys_i.systemMatrixTag();
    for (unsigned int j = 0; j < n_sys; ++j)
    {
      if (i == j)
        continue;

      const std::pair<unsigned int, unsigned int> key{i, j};
      auto & J_ij = *libmesh_map_find(_off_diag_mats, key);
      sys_i.associateMatrixToTag(J_ij, sys_tag_i);
      _fe_problem.setJacobianBlockContext(i, j);
      sys_i.computeJacobianTags({sys_tag_i});
      sys_i.disassociateMatrixFromTag(J_ij, sys_tag_i);
    }
  }
}

void
NewtonSNESExecutor::registerFieldSplitIS(SNES snes, unsigned int n_sys)
{
  KSP ksp;
  PC pc;
  auto comm = PetscObjectComm((PetscObject)snes);
  LibmeshPetscCallA(comm, SNESGetKSP(snes, &ksp));
  LibmeshPetscCallA(comm, KSPGetPC(ksp, &pc));

  Mat J;
  LibmeshPetscCallA(comm, SNESGetJacobian(snes, &J, nullptr, nullptr, nullptr));
  std::vector<IS> row_is(n_sys);
  LibmeshPetscCallA(comm, MatNestGetISs(J, row_is.data(), nullptr));

  for (unsigned int i = 0; i < n_sys; ++i)
    LibmeshPetscCallA(comm, PCFieldSplitSetIS(pc, std::to_string(i).c_str(), row_is[i]));
}

PetscErrorCode
NewtonSNESExecutor::outerResidualCallback(SNES /*snes*/, Vec /*x*/, Vec /*f*/, void * ctx)
{
  PetscFunctionBegin;
  auto * ex = static_cast<NewtonSNESExecutor *>(ctx);
  const unsigned int n_sys = ex->_fe_problem.numNonlinearSystems();

  // x is a VecNest whose sub-Vecs are the per-system solution Vecs (updated in-place by SNES).
  // f is a VecNest whose sub-Vecs are the per-system RHS Vecs -- filled by computeResidual below.
  for (unsigned int i = 0; i < n_sys; ++i)
  {
    auto & sys_i = ex->_fe_problem.getNonlinearSystemBase(i);
    ex->_fe_problem.setCurrentNonlinearSystem(i);
    sys_i.computeResidual(sys_i.RHS(), sys_i.residualVectorTag());
  }
  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
NewtonSNESExecutor::outerJacobianCallback(SNES /*snes*/, Vec /*x*/, Mat A, Mat /*P*/, void * ctx)
{
  PetscFunctionBegin;
  auto * ex = static_cast<NewtonSNESExecutor *>(ctx);
  const unsigned int n_sys = ex->_fe_problem.numNonlinearSystems();

  for (unsigned int i = 0; i < n_sys; ++i)
  {
    auto & sys_i = ex->_fe_problem.getNonlinearSystemBase(i);
    ex->_fe_problem.setJacobianBlockContext(i, i);
    auto & J_ii = static_cast<libMesh::ImplicitSystem &>(sys_i.system()).get_system_matrix();
    sys_i.computeJacobian(J_ii, {sys_i.systemMatrixTag()});
  }

  ex->assembleOffDiagJacobian();

  PetscCall(MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY));
  PetscCall(MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY));

  PetscFunctionReturn(PETSC_SUCCESS);
}
