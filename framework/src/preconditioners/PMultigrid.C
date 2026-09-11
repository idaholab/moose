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

namespace
{
PetscErrorCode
finePCSetUp(PC pc)
{
  void * ctx;
  LibmeshPetscCallQ(PCShellGetContext(pc, &ctx));
  static_cast<NonlinearSystemBase *>(ctx)->setupKokkosEntityBlockSmoother();
  return LIBMESH_PETSC_SUCCESS;
}

PetscErrorCode
finePCApply(PC pc, Vec r, Vec x)
{
  void * ctx;
  LibmeshPetscCallQ(PCShellGetContext(pc, &ctx));
  static_cast<NonlinearSystemBase *>(ctx)->applyKokkosEntityBlockSmoother(r, x);
  return LIBMESH_PETSC_SUCCESS;
}
} // namespace

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

  MooseEnum smoother("point_jacobi entity_block", "entity_block");

  params.addParam<MooseEnum>(
      "smoother",
      smoother,
      "The smoother applied on each level that is smoothed rather than solved. 'entity_block', the "
      "default, inverts the block of degrees of freedom each mesh entity carries -- an element's "
      "interior modes, and each shared face, edge and vertex -- which reaches the modes a point "
      "smoother cannot damp and the coarse spaces do not represent. 'point_jacobi' reads the "
      "operator "
      "diagonal alone, which at high order ignores the coupling among the many basis functions one "
      "entity carries. The block smoother costs between a tenth and a quarter more per "
      "application, "
      "and is the faster of the two in time to solution from order four upward: on a 16 by 16 mesh "
      "of "
      "biquadratic elements it takes about a quarter of the time at order four and a fortieth of "
      "it at "
      "order eight, against a fifth more time at orders two and three, where a hierarchic entity "
      "carries a single mode and the two smoothers coincide.");

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
      "verify_operator_conditioning",
      false,
      "Whether to report the conditioning of the solver system's matrix-free operator: its norm, "
      "the "
      "norm of its inverse, and their product. The inverse is formed from a Cholesky factorization "
      "of the explicitly assembled operator and its norm is taken the same way the cycle's norm "
      "is, "
      "so the two are directly comparable and answer whether a large cycle norm reflects the "
      "operator it inverts. It costs one operator application and one triangular solve per degree "
      "of "
      "freedom, so it is a verification aid for small inputs.");

  params.addParam<bool>(
      "verify_preconditioner_symmetry",
      false,
      "Whether to check that the p-multigrid cycle this preconditioner applies is symmetric, which "
      "is what CG requires of it, by forming it explicitly and comparing it against its transpose. "
      "It costs one application of the whole cycle per degree of freedom of the solver system, so "
      "it is a verification aid for small inputs.");

  params.addParam<bool>(
      "verify_entity_blocks",
      false,
      "Whether to check the entity blocks of each level that assembles its operator against that "
      "assembled operator, entry by entry, after every linearization. A block on a partition "
      "boundary gathers only the contributions of its own process's elements, so the check is "
      "exact "
      "on one process and reports that difference on more.");

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
    _entity_block_smoother(getParam<MooseEnum>("smoother") == "entity_block"),
    _verify_level_operators(getParam<bool>("verify_level_operators")),
    _verify_level_transfers(getParam<bool>("verify_level_transfers")),
    _verify_level_galerkin(getParam<bool>("verify_level_galerkin")),
    _verify_level_matrices(getParam<bool>("verify_level_matrices")),
    _verify_entity_blocks(getParam<bool>("verify_entity_blocks")),
    _verify_preconditioner_symmetry(getParam<bool>("verify_preconditioner_symmetry")),
    _verify_operator_conditioning(getParam<bool>("verify_operator_conditioning")),
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

  // The coarsest level is solved rather than smoothed, so it needs no blocks; the orders ascend, so
  // that is the first
  if (_entity_block_smoother)
    for (const auto i : index_range(_levels))
      if (i)
      {
        _levels[i]->initEntityBlocks();

        _console << "  p = " << _levels[i]->order() << ": " << _levels[i]->numEntityBlocks()
                 << " entity blocks, largest " << _levels[i]->maxEntityBlockSize() << " dofs\n";
      }

  if (_entity_block_smoother)
  {
    _nl.initKokkosEntityBlockSmoother();

    _console << "  fine: " << _nl.numKokkosEntityBlocks() << " entity blocks, largest "
             << _nl.maxKokkosEntityBlockSize() << " dofs\n";
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

      if (_entity_block_smoother)
        _levels[i]->setupBlockSmootherPC(smoother_pc);
      else
        LibmeshPetscCall(PCSetType(smoother_pc, PCJACOBI));
    }
    else
    {
      // Factor the coarsest level directly. A multigrid preconditioner has to be a fixed linear
      // operator, because the outer Krylov method builds its space from repeated applications and
      // relates its recurrence residual to the true one on the assumption that the operator does
      // not change between them. A direct factorization is such an operator exactly, and it is
      // affordable because this is the coarsest level of a p-hierarchy: order one on the fine mesh.
      //
      // Iterating this level instead fails whichever way the iteration is stopped, which is why the
      // obvious cheaper alternatives are rejected here. Stopping on a relative tolerance makes the
      // work, and so the operator, depend on the right-hand side. Stopping after a fixed number of
      // iterations fixes the work but not the operator, because a Krylov method builds its
      // polynomial from the Krylov space of the vector it is given and so remains a nonlinear
      // function of that vector however many steps it runs. Either way the outer Krylov method
      // reports a recurrence residual its true residual does not match, and a linear problem takes
      // several Newton steps.
      //
      // No solver package is named, so PETSc selects one: in parallel that is whichever
      // distributed factorization the build provides, and on one process its own LU.
      // '-mg_coarse_pc_factor_mat_solver_type' overrides the choice.
      LibmeshPetscCall(KSPSetType(smoother, KSPPREONLY));
      LibmeshPetscCall(PCSetType(smoother_pc, PCLU));
    }

    // Read the level's own options last, so that everything set above is a default a user can
    // override. PCMG gives each level's solver its own options prefix -- mg_coarse_ for the
    // coarsest, mg_levels_<level>_ for the rest, and mg_levels_ for all of them at once -- and
    // without this call those prefixes are accepted on the command line and silently ignored.
    LibmeshPetscCall(KSPSetFromOptions(smoother));

    // The interpolation from level i to the next finer level (i + 1, or the solver system if i is
    // the finest of _levels) is this level's own transfer, per PCMGSetInterpolation's convention
    LibmeshPetscCall(PCMGSetInterpolation(pc, i + 1, _levels[i]->interpolationMat()));
  }

  KSP fine_smoother;
  LibmeshPetscCall(PCMGGetSmoother(pc, n_levels - 1, &fine_smoother));
  LibmeshPetscCall(KSPSetType(fine_smoother, KSPCHEBYSHEV));
  PC fine_smoother_pc;
  LibmeshPetscCall(KSPGetPC(fine_smoother, &fine_smoother_pc));

  // The finest level is the solver system itself, so its blocks and its residual come from the
  // system rather than from a level of the hierarchy
  if (_entity_block_smoother)
  {
    LibmeshPetscCall(PCSetType(fine_smoother_pc, PCSHELL));
    LibmeshPetscCall(PCShellSetContext(fine_smoother_pc, &_nl));
    LibmeshPetscCall(PCShellSetName(fine_smoother_pc, "entity-block Schwarz"));
    LibmeshPetscCall(PCShellSetSetUp(fine_smoother_pc, finePCSetUp));
    LibmeshPetscCall(PCShellSetApply(fine_smoother_pc, finePCApply));
  }
  else
    LibmeshPetscCall(PCSetType(fine_smoother_pc, PCJACOBI));

  LibmeshPetscCall(KSPSetFromOptions(fine_smoother));
}

#endif
