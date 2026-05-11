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
#include "NonlinearSystemBase.h"

#include "libmesh/libmesh.h"
#include "libmesh/petsc_solver_exception.h"
#include "libmesh/implicit_system.h"

#include "libmesh/petsc_matrix.h"
#include <petscsnes.h>
#include <petscsys.h>

registerMooseObject("MooseApp", NonlinearPreconditioning);

InputParameters
NonlinearPreconditioning::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("NonlinearPreconditioning");
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
  // Don't use libmesh macros which can throw
  if (_npc_snes)
    PetscCallAbort(this->comm().get(), SNESDestroy(&_npc_snes));
  if (_mat_nest)
    PetscCallAbort(this->comm().get(), MatDestroy(&_mat_nest));
}

void
NonlinearPreconditioning::initialSetup()
{
  setupNPC();
  allocateOffDiagMats();
  // Per-solve NPC wiring and MatNest Jacobian setup is done in wireToSNES(), called from
  // FEProblemBase::solve() before each outer solve.  This is necessary because libMesh
  // destroys and recreates the SNES after every solve(), so SNESSetNPC and SNESSetJacobian
  // must be called again on the fresh SNES each time.
}

void
NonlinearPreconditioning::wireToSNES(unsigned int sys_num)
{
  SNES outer_snes = _fe_problem.getNonlinearSystem(sys_num).getSNES();
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESSetNPC(outer_snes, _npc_snes));
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESSetNPCSide(outer_snes, PC_LEFT));

  // Build / rebuild the MatNest (sub-mat pointers change if libMesh recreated the matrices)
  buildMatNest(sys_num);

  // Install the MatNest as the outer Jacobian operator.
  LibmeshPetscCallA(_fe_problem.comm().get(),
                    SNESSetJacobian(outer_snes, _mat_nest, _mat_nest, outerJacobianCallback, this));

  // Register per-system ISes with the outer PC for fieldsplit configuration.
  registerFieldSplitIS(outer_snes, _fe_problem.numNonlinearSystems());

  // Apply any PETSc options stored under the sub-block-name prefix (e.g. -<name>_pc_type).
  LibmeshPetscCallA(_fe_problem.comm().get(),
                    SNESSetOptionsPrefix(outer_snes, (name() + "_").c_str()));
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESSetFromOptions(outer_snes));
}

void
NonlinearPreconditioning::setupNPC()
{
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESCreate(_fe_problem.comm().get(), &_npc_snes));
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESSetType(_npc_snes, SNESSHELL));
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESShellSetSolve(_npc_snes, npcShellSolve));
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESSetApplicationContext(_npc_snes, this));
}

void
NonlinearPreconditioning::allocateOffDiagMats()
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
      // Conservative nonzero estimate; allow dynamic allocation to avoid sparsity detection.
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
NonlinearPreconditioning::buildMatNest(unsigned int /*outer_sys_num*/)
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
    // Diagonal block: system's Jacobian matrix.  Cannot use getMatrix(systemMatrixTag()) here
    // because that tag is only transiently associated during computeJacobian; access the
    // underlying libMesh matrix directly instead.
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
NonlinearPreconditioning::assembleOffDiagJacobian()
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

      // Swap J_ij in for the system Jacobian so that existing kernel paths
      // (which contribute to the SYSTEM tag) write to J_ij instead of J_ii.
      sys_i.associateMatrixToTag(J_ij, sys_tag_i);

      // Set the AD seeding context: ISys = i (residual), JSys = j (DOFs to seed).
      _fe_problem.setJacobianBlockContext(i, j);

      // Run system i's kernel assembly.  computeJacobianTags zeros J_ij and
      // accumulates contributions via the (now swapped-in) SYSTEM tag.
      sys_i.computeJacobianTags({sys_tag_i});

      sys_i.disassociateMatrixFromTag(J_ij, sys_tag_i);
    }
  }
}

void
NonlinearPreconditioning::registerFieldSplitIS(SNES outer_snes, unsigned int n_sys)
{
  KSP ksp;
  PC pc;
  auto comm = PetscObjectComm((PetscObject)outer_snes);
  LibmeshPetscCallA(comm, SNESGetKSP(outer_snes, &ksp));
  LibmeshPetscCallA(comm, KSPGetPC(ksp, &pc));

  // The MatNest stores one IS per row block.  Retrieve them and hand them to fieldsplit.
  IS * row_is = nullptr;
  Mat J;
  LibmeshPetscCallA(comm, SNESGetJacobian(outer_snes, &J, nullptr, nullptr, nullptr));
  LibmeshPetscCallA(comm, MatNestGetISs(J, row_is, nullptr));

  for (unsigned int i = 0; i < n_sys; ++i)
    LibmeshPetscCallA(comm, PCFieldSplitSetIS(pc, std::to_string(i).c_str(), row_is[i]));
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

PetscErrorCode
NonlinearPreconditioning::outerJacobianCallback(SNES, Vec /*x*/, Mat A, Mat /*P*/, void * ctx)
{
  PetscFunctionBegin;
  auto * npc = static_cast<NonlinearPreconditioning *>(ctx);
  const unsigned int n_sys = npc->_fe_problem.numNonlinearSystems();

  // Assemble diagonal blocks J_ii for each system.  computeJacobian() handles the transient
  // tag association internally; computeJacobianTags() alone would silently skip because the
  // system matrix tag is unassociated outside of computeJacobian().
  for (unsigned int i = 0; i < n_sys; ++i)
  {
    auto & sys_i = npc->_fe_problem.getNonlinearSystemBase(i);
    npc->_fe_problem.setJacobianBlockContext(i, i);
    auto & J_ii = static_cast<libMesh::ImplicitSystem &>(sys_i.system()).get_system_matrix();
    sys_i.computeJacobian(J_ii, {sys_i.systemMatrixTag()});
  }

  // Assemble off-diagonal blocks J_ij (i != j).
  npc->assembleOffDiagJacobian();

  // Reassemble the MatNest to pick up any pointer changes.
  PetscCall(MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY));
  PetscCall(MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY));

  PetscFunctionReturn(PETSC_SUCCESS);
}
