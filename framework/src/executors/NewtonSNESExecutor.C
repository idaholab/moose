//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NewtonSNESExecutor.h"
#include "Convergence.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "NonlinearSystemBase.h"
#include "SNESNPCExecutor.h"

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
  params += Moose::PetscSupport::newtonKrylovParams();
  params.addRequiredParam<std::vector<NonlinearSystemName>>(
      "nonlinear_system_names", "Name of the nonlinear systems this executor targets.");
  params.addParam<std::vector<ConvergenceName>>(
      "convergence_names",
      "Convergence object name(s) for each nonlinear system. If provided, must have the same "
      "length as 'nonlinear_system_names'.");
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
  {
    const auto sys_num = _fe_problem.nlSysNum(nl_sys_name);
    _nl_sys_nums.push_back(sys_num);
    const auto & sys = _fe_problem.getNonlinearSystemBase(sys_num);
    Moose::PetscSupport::storePetscOptions(_fe_problem, sys.prefix(), *this);
    auto & solver_params = _fe_problem.solverParams(sys_num);
    solver_params._prefix = sys.prefix();
    solver_params._solver_sys_num = sys_num;
  }

  if (_nl_sys_nums.size() > 0)
  {
    // Need full Jacobian in order to actually compute cross-system coupling
    _fe_problem.setCoupling(Moose::COUPLING_FULL);
    for (const auto i : index_range(_nl_sys_nums))
      for (const auto j : index_range(_nl_sys_nums))
      {
        if (i == j)
          continue;
        const TagID tag =
            _fe_problem.addMatrixTag("NPC_J_" + std::to_string(i) + "_" + std::to_string(j));
        _off_diag_mats[{i, j}] = {nullptr, tag};
      }
  }

  if (isParamValid("convergence_names"))
  {
    const auto & conv_names = getParam<std::vector<ConvergenceName>>("convergence_names");
    if (conv_names.size() != nl_sys_names.size())
      paramError("convergence_names",
                 "Must have the same length as 'nonlinear_system_names' (",
                 nl_sys_names.size(),
                 ")");
    for (const auto i : index_range(nl_sys_names))
      _fe_problem.setNonlinearConvergence(nl_sys_names[i], conv_names[i]);
  }

  Moose::PetscSupport::setESLinearSolverParams(_fe_problem.es(), *this);
}

NewtonSNESExecutor::~NewtonSNESExecutor()
{
  if (_jac_shell)
    PetscCallAbort(this->comm().get(), MatDestroy(&_jac_shell));
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
  // _vec_sol uses independent (non-aliasing) sub-vecs so the outer Newton iterate is not
  // corrupted when the NPC sub-solve updates the libmesh solution vectors.
  std::vector<Vec> ind_sol_vecs(n_sys);
  std::vector<Vec> rhs_vecs(n_sys);
  for (unsigned int i = 0; i < n_sys; ++i)
  {
    auto & sys_i = _fe_problem.getNonlinearSystem(i);
    Vec libmesh_sol =
        cast_ptr<libMesh::PetscVector<libMesh::Number> *>(sys_i.system().solution.get())->vec();
    LibmeshPetscCallA(this->comm().get(), VecDuplicate(libmesh_sol, &ind_sol_vecs[i]));
    LibmeshPetscCallA(this->comm().get(), VecCopy(libmesh_sol, ind_sol_vecs[i]));
    rhs_vecs[i] = cast_ptr<libMesh::PetscVector<libMesh::Number> *>(&sys_i.RHS())->vec();
  }
  LibmeshPetscCallA(
      this->comm().get(),
      VecCreateNest(this->comm().get(), n_sys, nullptr, ind_sol_vecs.data(), &_vec_sol));
  for (unsigned int i = 0; i < n_sys; ++i)
    LibmeshPetscCallA(this->comm().get(), VecDestroy(&ind_sol_vecs[i]));
  LibmeshPetscCallA(this->comm().get(),
                    VecCreateNest(this->comm().get(), n_sys, nullptr, rhs_vecs.data(), &_vec_func));

  allocateOffDiagMats();
  buildMatNest();

  LibmeshPetscCallA(this->comm().get(), SNESCreate(this->comm().get(), &_snes));
  // Make sure to set type early because otherwise SNESSetFromOptions at the end of this function
  // will call SNESSetType which will forward to SNESCreate_NEWTONLS which will overwrite our NPC
  // default side choice
  LibmeshPetscCallA(this->comm().get(), SNESSetType(_snes, SNESNEWTONLS));
  LibmeshPetscCallA(this->comm().get(),
                    SNESSetFunction(_snes, _vec_func, outerResidualCallback, this));

  PetscInt M, N, m, n;
  LibmeshPetscCallA(this->comm().get(), MatGetSize(_mat_nest, &M, &N));
  LibmeshPetscCallA(this->comm().get(), MatGetLocalSize(_mat_nest, &m, &n));
  LibmeshPetscCallA(this->comm().get(),
                    MatCreateShell(this->comm().get(), m, n, M, N, this, &_jac_shell));
  LibmeshPetscCallA(this->comm().get(),
                    MatShellSetOperation(_jac_shell, MATOP_MULT, (PetscErrorCodeFn *)shellMatMult));

  LibmeshPetscCallA(this->comm().get(),
                    SNESSetJacobian(_snes, _jac_shell, _mat_nest, outerJacobianCallback, this));

  // Outer KSP needs no linear preconditioner: all preconditioning is at the NL level.
  KSP ksp;
  PC pc;
  LibmeshPetscCallA(this->comm().get(), SNESGetKSP(_snes, &ksp));
  LibmeshPetscCallA(this->comm().get(), KSPGetPC(ksp, &pc));
  LibmeshPetscCallA(this->comm().get(), PCSetType(pc, PCNONE));

  // Drive ||x - NPC(x)|| to zero (ASPIN-style fixed-point convergence).
  LibmeshPetscCallA(this->comm().get(), SNESSetNPCSide(_snes, PC_LEFT));
  LibmeshPetscCallA(this->comm().get(), SNESSetFunctionType(_snes, SNES_FUNCTION_PRECONDITIONED));

  // Set options prefix and set from options
  LibmeshPetscCallA(this->comm().get(), SNESSetOptionsPrefix(_snes, (this->name() + "_").c_str()));
  LibmeshPetscCallA(this->comm().get(), SNESSetFromOptions(_snes));

  // Other setup
  for (const auto nl_sys_i_num : _nl_sys_nums)
    for (const auto nl_sys_j_num : _nl_sys_nums)
      if (nl_sys_i_num != nl_sys_j_num)
        _fe_problem.setFullCoupling(nl_sys_i_num, nl_sys_j_num, {});

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

    PetscInt iter;
    LibmeshPetscCallA(
        this->comm().get(),
        SNESGetIterationNumber(_fe_problem.getNonlinearSystem(nl_sys_num).getSNES(), &iter));
    const auto & conv_name = _fe_problem.getNonlinearConvergenceNames()[nl_sys_num];
    const auto status = _fe_problem.getConvergence(conv_name).checkConvergence(iter);
    if (status == Convergence::MooseConvergenceStatus::CONVERGED)
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

  // Ensure the PETSc options database is populated before SNESSetFromOptions() runs inside
  // setupSNES(). Without this, petscSetOptions() called from the first FEProblemBase::solve()
  // during the NPC sub-solve would clear and re-insert options with used=false, causing PETSc to
  // report options like -outer_snes_monitor as unused at finalization even though they are active.
  _fe_problem.insertPetscOptionsIfNeeded();

  // SNES setup also calls buildMatNest()
  if (!_snes_setup_done)
    setupSNES();
  else
    buildMatNest();

  LibmeshPetscCallA(this->comm().get(), SNESSetNPC(_snes, _npc_executor->getSNES()));

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
  for (const auto i : index_range(_nl_sys_nums))
  {
    auto & sys_i = _fe_problem.getNonlinearSystemBase(_nl_sys_nums[i]);
    for (const auto j : index_range(_nl_sys_nums))
    {
      if (i == j)
        continue;

      auto & sys_j = _fe_problem.getNonlinearSystemBase(_nl_sys_nums[j]);

      const auto m = sys_i.system().n_dofs();
      const auto n = sys_j.system().n_dofs();
      const auto m_l = sys_i.system().n_local_dofs();
      const auto n_l = sys_j.system().n_local_dofs();

      auto mat = std::make_unique<libMesh::PetscMatrix<libMesh::Number>>(_fe_problem.comm());
      // We'll allow the matrix to be hash table assembled
      mat->init_without_preallocation(m, n, m_l, n_l, 1);
      mat->finish_initialization();
      LibmeshPetscCallA(_fe_problem.comm().get(),
                        MatSetOption(mat->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_TRUE));

      const std::pair<unsigned int, unsigned int> key{i, j};
      auto & [our_mat, tag] = libmesh_map_find(_off_diag_mats, key);
      sys_i.associateMatrixToTag(*mat, tag);
      our_mat = std::move(mat);
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
  for (const auto i : make_range(n_sys))
  {
    auto & sys_i = _fe_problem.getNonlinearSystem(i);
    auto & J_ii =
        static_cast<libMesh::PetscMatrix<libMesh::Number> &>(sys_i.sys().get_system_matrix());
    sub_mats[i * n_sys + i] = J_ii.mat();

    for (unsigned int j = 0; j < n_sys; ++j)
    {
      if (i == j)
        continue;
      const std::pair<unsigned int, unsigned int> key{i, j};
      sub_mats[i * n_sys + j] = libmesh_map_find(_off_diag_mats, key).mat->mat();
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
  const auto n_sys = _nl_sys_nums.size();
  for (const auto i : make_range(n_sys))
  {
    const auto nl_sys_i_num = _nl_sys_nums[i];
    auto & sys_i = _fe_problem.getNonlinearSystemBase(nl_sys_i_num);
    for (const auto j : make_range(n_sys))
    {
      if (i == j)
        // We already computed and will re-use the sub-SNES diagonal Jacobian blocks
        continue;

      const auto nl_sys_j_num = _nl_sys_nums[j];
      _fe_problem.setJacobianBlockContext(nl_sys_i_num, nl_sys_j_num);
      const std::pair<unsigned int, unsigned int> key{i, j};
      auto & [J_ij, tag] = libmesh_map_find(_off_diag_mats, key);
      _fe_problem.computeJacobianTag(*sys_i.system().current_local_solution, *J_ij, tag);
    }
  }
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
NewtonSNESExecutor::shellMatMult(Mat m, Vec X, Vec Y)
{
  void * ctx;

  PetscFunctionBegin;
  PetscCall(MatShellGetContext(m, &ctx));
  auto * ex = static_cast<NewtonSNESExecutor *>(ctx);
  mooseAssert(ex->_npc_executor,
              "Should not be using a shell matrix without a nonlinear preconditioner");
  PetscCall(ex->_npc_executor->applyBA(ex->_mat_nest, X, Y));
  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
NewtonSNESExecutor::outerJacobianCallback(SNES /*snes*/, Vec /*x*/, Mat /*A*/, Mat P, void * ctx)
{
  KSP sub_ksp;

  PetscFunctionBegin;
  auto * ex = static_cast<NewtonSNESExecutor *>(ctx);

  for (const auto nl_sys_num : ex->_nl_sys_nums)
  {
    auto & moose_sys = ex->_fe_problem.getNonlinearSystem(nl_sys_num);
    auto & lm_sys = moose_sys.sys();
    auto & J_ii = lm_sys.get_system_matrix();
    // We'll assume that the sub-solves converged tightly enough that we can assume solution
    // constraints are already enforced. And System::update() is called at the end of sub-solves
    // such that we should already have current_local_solution in a good state
    ex->_fe_problem.computeJacobian(*lm_sys.current_local_solution, J_ii, nl_sys_num);
    auto & dof_map = lm_sys.get_dof_map();
    if (dof_map.n_constrained_dofs())
    {
      // Even though our solution was constrained we stupidly don't apply our asymmetric constraints
      // at the element level so the Jacobian will be wrong without this call below
      dof_map.enforce_constraints_on_jacobian(lm_sys, &J_ii);
      J_ii.close();
    }
  }

  ex->assembleOffDiagJacobian();

  // P is the raw MatNest; A is the ASPIN shell (no assembly needed for shell).
  PetscCall(MatAssemblyBegin(P, MAT_FINAL_ASSEMBLY));
  PetscCall(MatAssemblyEnd(P, MAT_FINAL_ASSEMBLY));

  // Update per-field KSP operators from the freshly assembled diagonal blocks so that
  // shellMatMult reuses the current factorizations across all Krylov iterations.
  for (const auto i : index_range(ex->_nl_sys_nums))
  {
    Mat J_ii;
    PetscCall(MatNestGetSubMat(P, i, i, &J_ii));
    auto sub_snes = ex->_fe_problem.getNonlinearSystem(ex->_nl_sys_nums[i]).getSNES();
    PetscCall(SNESGetKSP(sub_snes, &sub_ksp));
    PetscCall(KSPSetOperators(sub_ksp, J_ii, J_ii));
  }

  PetscFunctionReturn(PETSC_SUCCESS);
}

SNES
NewtonSNESExecutor::getSNES()
{
  if (_nl_sys_nums.size() != 1)
    mooseError("Ambiguous call to getSNES()");

  // We cannot rely on caching this during some setup routine because libMesh destroys its SNES at
  // the end of each nonlinear solve. For similar reasons we cannot have _snes point to this because
  // then at destruction time we'll attempt to destroy through a pointer to garbage since the SNES
  // was already destroyed at the end of the solve
  return _fe_problem.getNonlinearSystem(_nl_sys_nums[0]).getSNES();
}

System &
NewtonSNESExecutor::getSystem()
{
  if (_nl_sys_nums.size() != 1)
    mooseError("Ambiguous call to getSystem()");

  return _fe_problem.getNonlinearSystem(_nl_sys_nums[0]).system();
}
