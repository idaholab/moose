//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/petsc_macro.h"

#include "PMultigrid.h"

#ifdef MOOSE_KOKKOS_ENABLED

#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"

#include "libmesh/petsc_nonlinear_solver.h"

registerMooseObjectAliased("MooseApp", PMultigrid, "PMG");

InputParameters
PMultigrid::validParams()
{
  InputParameters params = MoosePreconditioner::validParams();

  params.addClassDescription(
      "p-multigrid preconditioner for the Kokkos matrix-free partial-assembly Jacobian. Each "
      "coarse level contracts the fine level's quadrature-point Jacobian cache against a "
      "reduced-order basis on the same mesh and the same quadrature rule.");

  params.addRequiredParam<std::vector<unsigned int>>(
      "level_orders",
      "The polynomial order of each coarse level, ascending. The fine level is the solver system's "
      "own order and is not listed.");

  params.addParam<bool>(
      "verify_level_operators",
      false,
      "Whether to check each level's operator against its diagonal after every linearization, by "
      "applying the operator to a unit vector per degree of freedom. This costs one operator "
      "application per degree of freedom of every level, so it is a verification aid for small "
      "inputs.");

  params.addParam<bool>(
      "verify_level_transfers",
      false,
      "Whether to check that each level transfer restricts by the transpose of its prolongation, "
      "which is what makes the operator the hierarchy realizes on a coarse level the Galerkin "
      "operator of the fine linearization. The check costs one application of each direction of "
      "each transfer, at initial setup only.");

  params.addParam<bool>(
      "verify_level_galerkin",
      false,
      "Whether to check each level's operator against P^T A P after every linearization, where A "
      "is the operator of the next finer level and P is the transfer between the two. At the "
      "finest pair A is the matrix-free Jacobian of the solver system, so the check chains down "
      "the hierarchy and establishes that every level is consistent with the fine linearization. "
      "It costs one application of the next finer level's operator and one of each direction of "
      "the transfer, per level per linearization.");

  params.addParam<bool>(
      "verify_level_matrices",
      false,
      "Whether to check the assembled operator of each level that assembles one against the "
      "operator the level applies without a matrix, after every linearization. The check costs one "
      "application of each of the two, per assembled level per linearization.");

  params.addParam<bool>(
      "verify_operator_symmetry",
      false,
      "Whether to check that the solver system's own matrix-free operator, which serves as SNES's "
      "Amat, is symmetric after every linearization. This is what makes CG a valid outer Krylov "
      "accelerator over the hierarchy, so a solve that asks for CG should also ask for this check. "
      "It costs one operator application per degree of freedom of the solver system, so it is a "
      "verification aid for small inputs.");

  return params;
}

PMultigrid::PMultigrid(const InputParameters & parameters)
  : MoosePreconditioner(parameters),
    _level_orders(getParam<std::vector<unsigned int>>("level_orders")),
    _verify_level_operators(getParam<bool>("verify_level_operators")),
    _verify_level_transfers(getParam<bool>("verify_level_transfers")),
    _verify_level_galerkin(getParam<bool>("verify_level_galerkin")),
    _verify_level_matrices(getParam<bool>("verify_level_matrices")),
    _verify_operator_symmetry(getParam<bool>("verify_operator_symmetry"))
{
  if (_level_orders.empty())
    paramError("level_orders", "At least one coarse level is required.");

  for (const auto i : index_range(_level_orders))
    if (i && _level_orders[i] <= _level_orders[i - 1])
      paramError("level_orders", "The level orders must be strictly ascending.");

  auto & solver_params = _fe_problem.solverParams(_nl_sys_num);

  if (!solver_params._kokkos_matrix_free)
    mooseError(
        "The p-multigrid preconditioner's operators are the quadrature-point Jacobian cache "
        "contracted against each level's basis, so the solver system must be run in Kokkos "
        "matrix-free mode. Set 'use_kokkos_matrix_free_jacobian = true' in the Executioner.");

  // A p-multigrid preconditioner configures the solve's PETSc preconditioner itself, so the
  // matrix-free default that would otherwise apply (a bare Jacobi over the fine shell) is left to
  // that configuration
  solver_params._kokkos_p_multigrid = true;

  // The levels' systems have to exist before the equation systems are initialized, and their FE
  // types have to be registered before the Kokkos assembly caches reference shape data; the
  // preconditioner is constructed ahead of both. The coarsest level assembles its operator, which
  // is what a coarse solver of the cycle is applied to; the orders ascend, so that is the first.
  for (const auto i : index_range(_level_orders))
    _levels.push_back(std::make_unique<Moose::Kokkos::PLevelSpace>(_nl, _level_orders[i], !i));
}

void
PMultigrid::initialSetup()
{
  MoosePreconditioner::initialSetup();

  _console << "\np-multigrid levels for system '" << _nl.name() << "':\n";

  for (const auto & level : _levels)
  {
    // The levels' DOFs are distributed by now, so each level can build the device DOF layout its
    // vectors and its gather/scatter are indexed by
    level->init();

    _console << "  p = " << level->order() << ": " << level->system().n_dofs() << " dofs, "
             << level->numConstrainedDofs() << " constrained by nodal boundary conditions\n";
  }

  _console << "  fine: " << _nl.system().n_dofs() << " dofs\n" << std::endl;

  // A transfer indexes the DOF layout of both of its sides, so the transfers are built once every
  // level has one
  for (const auto i : index_range(_levels))
    _levels[i]->initTransfer(i + 1 < _levels.size() ? _levels[i + 1].get() : nullptr);

  if (_verify_level_transfers)
  {
    _console << "p-multigrid level transfers for system '" << _nl.name() << "':\n";

    for (const auto & level : _levels)
      level->verifyTransfer(_console);

    _console << std::endl;
  }
}

void
PMultigrid::setupSolver()
{
  SNES snes = _nl.getSNES();
  KSP ksp;
  LibmeshPetscCall(SNESGetKSP(snes, &ksp));
  PC pc;
  LibmeshPetscCall(KSPGetPC(ksp, &pc));

  // KSPGetPC copies libMesh's DM onto a newly created PC. Every operator, interpolation, and
  // smoother setting below is supplied explicitly, so the DM would only add unwanted work: with
  // PC_MG_GALERKIN_NONE, PCSetUp_MG restricts the SNES solution vector through the DM hierarchy
  // between levels, and libMesh's DM does not support the coarse levels that restriction needs
  LibmeshPetscCall(PCSetDM(pc, nullptr));

  // PCMG numbers levels from the coarsest (0) to the finest; the finest level is the solver
  // system itself, which is not one of _levels
  const auto n_levels = _levels.size() + 1;

  LibmeshPetscCall(PCSetType(pc, PCMG));
  LibmeshPetscCall(PCMGSetLevels(pc, n_levels, nullptr));
  // Every level's operator is supplied directly below, rather than formed by PETSc from the
  // interpolation and the next finer level's operator
  LibmeshPetscCall(PCMGSetGalerkin(pc, PC_MG_GALERKIN_NONE));

  for (const auto i : index_range(_levels))
  {
    KSP smoother;
    LibmeshPetscCall(PCMGGetSmoother(pc, i, &smoother));
    LibmeshPetscCall(
        KSPSetOperators(smoother, _levels[i]->operatorMat(), _levels[i]->operatorMat()));

    PC smoother_pc;
    LibmeshPetscCall(KSPGetPC(smoother, &smoother_pc));

    // The coarsest level is solved; every other level (including the finest, set below) is
    // smoothed. None of these operators carry matrix entries except the assembled coarsest one, so
    // the defaults below are the ones that apply to a shell: Jacobi rather than SOR, and algebraic
    // multigrid rather than a direct factorization.
    if (i)
    {
      LibmeshPetscCall(KSPSetType(smoother, KSPCHEBYSHEV));
      LibmeshPetscCall(PCSetType(smoother_pc, PCJACOBI));
    }
    else
    {
      // Iterate the coarse level to convergence rather than applying one multigrid cycle to it. A
      // single cycle leaves a coarse-grid correction the outer Krylov method then has to repair on
      // every subsequent iteration, which costs iteration counts that grow with the fine order: on
      // a 4x4 mesh at order 8 over levels 1, 2 and 4, one cycle needs 144 iterations to reach a
      // linear tolerance of 1e-8 where a converged coarse solve needs 88, matching what a direct
      // factorization of that level achieves. CG is the accelerator here because the coarsest
      // level's assembled operator is symmetric, which verifyKokkosLevelMatrices() measures.
      LibmeshPetscCall(KSPSetType(smoother, KSPCG));
      LibmeshPetscCall(KSPSetTolerances(smoother, 1e-10, PETSC_DEFAULT, PETSC_DEFAULT, 200));
      LibmeshPetscCall(PCSetType(smoother_pc, PCGAMG));
    }

    // The interpolation from level i to the next finer level (i + 1, or the solver system if i is
    // the finest of _levels) is this level's own transfer, per PCMGSetInterpolation's convention
    LibmeshPetscCall(PCMGSetInterpolation(pc, i + 1, _levels[i]->interpolationMat()));
  }

  KSP fine_smoother;
  LibmeshPetscCall(PCMGGetSmoother(pc, n_levels - 1, &fine_smoother));
  LibmeshPetscCall(KSPSetType(fine_smoother, KSPCHEBYSHEV));
  PC fine_smoother_pc;
  LibmeshPetscCall(KSPGetPC(fine_smoother, &fine_smoother_pc));
  LibmeshPetscCall(PCSetType(fine_smoother_pc, PCJACOBI));
}

#endif
