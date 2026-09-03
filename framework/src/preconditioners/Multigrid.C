//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/petsc_macro.h"
#include "Multigrid.h"

#include "FEProblem.h"
#include "NonlinearSystemBase.h"
#include "MultigridLevelsAction.h"
#include "PetscSupport.h"

#include "libmesh/petsc_matrix_base.h"
#include "libmesh/fe_interface.h"
#include "libmesh/nonlinear_implicit_system.h"

using namespace libMesh;

namespace
{
Mat
petscMat(SystemBase & sys, TagID tag)
{
  return cast_ptr<PetscMatrixBase<Number> *>(&sys.getMatrix(tag))->mat();
}
}

registerMooseObjectAliased("MooseApp", Multigrid, "MG");

InputParameters
Multigrid::validParams()
{
  InputParameters params = MoosePreconditioner::validParams();
  params.addClassDescription(
      "p-multigrid preconditioner for the Kokkos matrix-free partial-assembly Jacobian-vector "
      "product. See idaholab/moose#33644.");

  params.addParam<std::vector<unsigned int>>(
      "orders",
      {1, 2},
      "The LAGRANGE polynomial order of each level, ascending from level 0 "
      "(coarsest, solved with an assembled matrix and an algebraic multigrid coarse solve) to "
      "the last entry, which must equal the fine variable's own order.");
  params.addParam<std::string>(
      "smoother_ksp_type", "chebyshev", "The PETSc KSP type used by every level's smoother.");
  params.addParam<std::string>(
      "smoother_pc_type", "jacobi", "The PETSc PC type used by every level's smoother.");
  params.addParam<unsigned int>(
      "smoother_its", 2, "The number of smoothing iterations performed on every level.");
  params.addParam<std::string>(
      "coarse_pc_type", "hypre", "The PETSc PC type used for the level-0 coarse solve.");

  return params;
}

Multigrid::Multigrid(const InputParameters & parameters)
  : MoosePreconditioner(parameters),
    _orders(getParam<std::vector<unsigned int>>("orders")),
    _smoother_ksp_type(getParam<std::string>("smoother_ksp_type")),
    _smoother_pc_type(getParam<std::string>("smoother_pc_type")),
    _smoother_its(getParam<unsigned int>("smoother_its")),
    _coarse_pc_type(getParam<std::string>("coarse_pc_type")),
    _fine_var_name(_nl.system().variable_name(0)),
    _configured(false)
{
  if (_orders.size() < 2)
    paramError("orders", "Multigrid requires at least two levels.");

  // The coarse levels' systems were created by MultigridLevelsAction, at the "meta_action" task,
  // long before this preconditioner was constructed
  _level_systems.reserve(_orders.size());
  for (const auto l : make_range(_orders.size() - 1))
  {
    const auto sys_name = MultigridLevelsAction::levelSystemName(_fine_var_name, l);
    _level_systems.push_back(&_fe_problem.getNonlinearSystemBase(_fe_problem.nlSysNum(sys_name)));
  }
  _level_systems.push_back(&_nl);
}

void
Multigrid::initialSetup()
{
  MoosePreconditioner::initialSetup();

  // Every level above 0 gets a matrix-free shell installed as its tagged system matrix, in place
  // of the ordinary assembled matrix EquationSystems::init() already created for it (at the
  // "init_problem" task, long since passed by initialSetup() time) -- replacing rather than
  // preempting that matrix, since sizing the shell from the DofMap requires dofs already
  // distributed, which is also done as part of that same EquationSystems::init() call with no
  // task boundary in between. setupKokkosMatrixFreeJacobian() then finds the shell already
  // installed and registers MatMult/MatGetDiagonal on it instead of the single-system Amat-only
  // path it falls back to otherwise.
  for (const auto l : make_range(std::size_t(1), _level_systems.size()))
  {
    _level_systems[l]->addShellMatrix(_level_systems[l]->systemMatrixTag());
    _level_systems[l]->setupKokkosMatrixFreeJacobian();
  }

  _prolongation.resize(_orders.size() - 1);
  for (const auto l : make_range(std::size_t(1), _level_systems.size()))
    _prolongation[l - 1] = buildProlongation(l);

  // For linear diffusion the level-0 operator is solution-independent (Milestone 1's own scope,
  // see the design's "Level solution state" note), so its Jacobian is assembled once, here,
  // rather than on every Newton iteration; PCMG is wired onto the fine system's SNES right after,
  // since level 0's matrix must exist before KSPSetOperators() can point the coarse solve at it.
  // A genuine per-iteration hook is deferred along with nonlinear multigrid.
  updateLevelOperators();
}

std::unique_ptr<PetscMatrix<Number>>
Multigrid::buildProlongation(unsigned int level) const
{
  auto & coarse_sys = *_level_systems[level - 1];
  auto & fine_sys = *_level_systems[level];

  const auto & mesh = _fe_problem.mesh().getMesh();

  const auto & coarse_dof_map = coarse_sys.system().get_dof_map();
  const auto & fine_dof_map = fine_sys.system().get_dof_map();

  const auto coarse_var = coarse_sys.getVariable(0, 0).number();
  const auto fine_var = fine_sys.getVariable(0, 0).number();

  const auto coarse_fe_type = coarse_dof_map.variable_type(coarse_var);

  auto p = std::make_unique<PetscMatrix<Number>>(_fe_problem.comm());
  // A generous, fixed over-estimate of the nonzeros per row (every coarse dof of every element
  // incident on a fine dof) -- exact preallocation from element counts is deferred; PETSc
  // reallocates rather than erroring if this is exceeded.
  p->init(fine_dof_map.n_dofs(),
          coarse_dof_map.n_dofs(),
          fine_dof_map.n_local_dofs(),
          coarse_dof_map.n_local_dofs(),
          /* n_nz = */ 64,
          /* n_oz = */ 64);

  std::vector<dof_id_type> fine_dofs, coarse_dofs;

  // A modal (non-nodal) family would replace this collocation with an element-local dense solve;
  // deferred along with HIERARCHIC. LAGRANGE-to-LAGRANGE collocation needs no such solve because
  // every fine dof is itself a nodal point at which the coarse shape functions can be evaluated
  // directly.
  //
  // Dirichlet-constrained fine rows are not zeroed here (see the design's Verification step 3);
  // this is a known simplification of Milestone 1, tracked as follow-up work.
  for (const auto * elem : mesh.active_local_element_ptr_range())
  {
    fine_dof_map.dof_indices(elem, fine_dofs, fine_var);
    coarse_dof_map.dof_indices(elem, coarse_dofs, coarse_var);

    for (const auto i : index_range(fine_dofs))
    {
      const Point x_i = elem->point(i);
      const Point ref = FEInterface::inverse_map(mesh.mesh_dimension(), coarse_fe_type, elem, x_i);

      for (const auto j : index_range(coarse_dofs))
      {
        const Real c_ij = FEInterface::shape(coarse_fe_type, elem, j, ref);
        if (c_ij != 0.0)
          p->set(fine_dofs[i], coarse_dofs[j], c_ij);
      }
    }
  }

  p->close();

  return p;
}

Mat
Multigrid::levelMat(unsigned int level) const
{
  return petscMat(*_level_systems[level], _level_systems[level]->systemMatrixTag());
}

void
Multigrid::updateLevelOperators()
{
  // Level 0 is an ordinary assembled system; nothing else ever triggers its Jacobian assembly.
  // computeJacobian() disassociates the matrix from its tag once it returns, so the tag is
  // re-associated afterward -- permanently, since nothing else will ever reassemble this
  // solution-independent (linear) operator -- for PCMG's coarse solve to find later.
  auto & level0 = *_level_systems[0];
  auto & level0_mat = level0.addMatrix(level0.systemMatrixTag());
  level0.computeJacobian(level0_mat);
  level0.associateMatrixToTag(level0_mat, level0.systemMatrixTag());

  if (!_configured)
    configure();
}

void
Multigrid::configure()
{
  const auto comm = _nl.comm().get();

  SNES snes = _nl.getSNES();
  KSP ksp;
  PC pc;
  LibmeshPetscCallA(comm, SNESGetKSP(snes, &ksp));
  LibmeshPetscCallA(comm, KSPGetPC(ksp, &pc));

  // All level operators are supplied explicitly below; without this, PCMG's setup tries to
  // auto-restrict the placeholder DM libMesh attaches to every PETSc nonlinear solve down to
  // each level, which fails since that DM is a bare DMSHELL that never implements
  // DMCreateGlobalVector. Detaching it here is safe: MOOSE's own residual/Jacobian callbacks
  // never go through it.
  LibmeshPetscCallA(comm, KSPSetDMActive(ksp, KSP_DMACTIVE_ALL, PETSC_FALSE));
  LibmeshPetscCallA(comm, PCSetDM(pc, nullptr));

  LibmeshPetscCallA(comm, PCSetType(pc, PCMG));
  LibmeshPetscCallA(comm, PCMGSetLevels(pc, _level_systems.size(), nullptr));
  LibmeshPetscCallA(comm, PCMGSetGalerkin(pc, PC_MG_GALERKIN_NONE));
  LibmeshPetscCallA(comm, PCMGSetType(pc, PC_MG_MULTIPLICATIVE));
  LibmeshPetscCallA(comm, PCMGSetCycleType(pc, PC_MG_CYCLE_V));

  for (const auto l : make_range(std::size_t(1), _level_systems.size()))
    LibmeshPetscCallA(comm, PCMGSetInterpolation(pc, l, _prolongation[l - 1]->mat()));

  for (const auto l : make_range(std::size_t(1), _level_systems.size()))
  {
    KSP lksp;
    PC lpc;
    LibmeshPetscCallA(comm, PCMGGetSmoother(pc, l, &lksp));

    const Mat a_l = levelMat(l);
    LibmeshPetscCallA(comm, KSPSetOperators(lksp, a_l, a_l));
    LibmeshPetscCallA(comm, KSPSetType(lksp, _smoother_ksp_type.c_str()));
    LibmeshPetscCallA(
        comm, KSPSetTolerances(lksp, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT, _smoother_its));
    LibmeshPetscCallA(comm, KSPGetPC(lksp, &lpc));
    LibmeshPetscCallA(comm, PCSetType(lpc, _smoother_pc_type.c_str()));
  }

  KSP ksp0;
  PC pc0;
  LibmeshPetscCallA(comm, PCMGGetCoarseSolve(pc, &ksp0));

  const Mat a_0 = levelMat(0);
  LibmeshPetscCallA(comm, KSPSetOperators(ksp0, a_0, a_0));
  LibmeshPetscCallA(comm, KSPSetType(ksp0, KSPPREONLY));
  LibmeshPetscCallA(comm, KSPGetPC(ksp0, &pc0));
  LibmeshPetscCallA(comm, PCSetType(pc0, _coarse_pc_type.c_str()));
  if (_coarse_pc_type == "hypre")
    LibmeshPetscCallA(comm, PCHYPRESetType(pc0, "boomeramg"));

  // So -mg_levels_*/-mg_coarse_* from the input can still override what we just configured
  LibmeshPetscCallA(comm, PCSetFromOptions(pc));

  _configured = true;
}
