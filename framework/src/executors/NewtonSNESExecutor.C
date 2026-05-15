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
      "Delegates to _fe_problem.solve() for a single nonlinear system, or builds a combined "
      "outer SNES with VecNest/MatNest when multiple systems are present (nl_preconditioning "
      "required in that path).");
  params += Moose::PetscSupport::kspRelatedParams();
  params.addRequiredParam<std::vector<NonlinearSystemName>>(
      "nonlinear_system_names", "Name of the nonlinear systems this executor targets.");
  return params;
}

NewtonSNESExecutor::NewtonSNESExecutor(const InputParameters & params) : SNESExecutor(params)
{
  // I don't actually know if this is possible with the parser like if the user passes an empty
  // string
  const auto & nl_sys_names = getParam<std::vector<NonlinearSystemName>>("nonlinear_system_names");
  if (nl_sys_names.empty())
    paramError("nonlinear_system_names", "Empty string passed?");

  for (const auto & nl_sys_name : nl_sys_names)
    _nl_sys_nums.push_back(_fe_problem.nlSysNum(nl_sys_name));

  Moose::PetscSupport::setESLinearSolverParams(_fe_problem.es(), *this);
}

NewtonSNESExecutor::~NewtonSNESExecutor()
{
  if (_mat_nest)
    PetscCallAbort(this->comm().get(), MatDestroy(&_mat_nest));
}

void
NewtonSNESExecutor::setupSNES()
{
  const auto n_sys = _nl_sys_nums.size();

  if (n_sys == 1)
    // We don't need to create a new SNES with Nest data structures. We'll just be leveraging the
    // libMesh solver's SNES
    return;

  // Build VecNest for solution and residual.
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
  LibmeshPetscCallA(this->comm().get(),
                    VecCreateNest(this->comm().get(), n_sys, nullptr, rhs_vecs.data(), &_vec_func));

  allocateOffDiagMats();
  buildMatNest();

  LibmeshPetscCallA(this->comm().get(), SNESCreate(this->comm().get(), &_snes));
  LibmeshPetscCallA(this->comm().get(),
                    SNESSetFunction(_snes, _vec_func, outerResidualCallback, this));
  LibmeshPetscCallA(this->comm().get(),
                    SNESSetJacobian(_snes, _mat_nest, _mat_nest, outerJacobianCallback, this));
  registerFieldSplitIS(_snes, n_sys);

  _snes_setup_done = true;
}

Executor::Result
NewtonSNESExecutor::run()
{
  auto & result = newResult();

  static const std::string solve_converged_msg = "Solve converged";
  static const std::string solve_didnt_converge_msg = "Solve failed to converge";

  if (_nl_sys_nums.size() == 1)
  {
    const auto nl_sys_num = _nl_sys_nums[0];

    // Wire the nonlinear preconditioner if we have it
    if (_npc_executor)
      LibmeshPetscCallA(this->comm().get(),
                        SNESSetNPC(_fe_problem.getNonlinearSystem(nl_sys_num).getSNES(),
                                   _npc_executor->getSNES()));

    _fe_problem.solve(nl_sys_num);
    if (_fe_problem.solverSystemConverged(nl_sys_num))
      result.pass(solve_converged_msg);
    else
      result.fail(solve_didnt_converge_msg);
    return result;
  }

  //
  // Multi-system: combined outer Newton with VecNest/MatNest.
  //

  if (!_npc_executor)
    mooseError("NewtonSNESExecutor: multiple nonlinear systems currently require "
               "'nl_preconditioning' to be set.");

  // SNES setup also calls buildMatNest()
  if (!_snes_setup_done)
    setupSNES();
  else
    buildMatNest();

  LibmeshPetscCallA(this->comm().get(), SNESSolve(_snes, nullptr, _vec_sol));

  SNESConvergedReason reason;
  LibmeshPetscCallA(this->comm().get(), SNESGetConvergedReason(_snes, &reason));
  if (reason > 0)
    result.pass(solve_converged_msg);
  else
    result.fail(solve_didnt_converge_msg);
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

  // x and f are VecNests whose sub-Vecs are the per-system solution and RHS Vecs.
  for (unsigned int i = 0; i < n_sys; ++i)
  {
    auto & sys_i = ex->_fe_problem.getNonlinearSystemBase(i);
    ex->_fe_problem.setCurrentNonlinearSystem(i);
    sys_i.computeResidualTag(sys_i.RHS(), sys_i.residualVectorTag());
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
