//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PMultigrid.h"

#ifdef MOOSE_KOKKOS_ENABLED

#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "KokkosLevelTransfer.h"

namespace
{
PetscErrorCode
mooseKokkosPreconditionerShellMult(Mat shell, Vec x, Vec y)
{
  void * ctx;
  LibmeshPetscCallQ(MatShellGetContext(shell, &ctx));
  LibmeshPetscCallQ(PCApply(static_cast<PC>(ctx), x, y));
  return LIBMESH_PETSC_SUCCESS;
}

PetscErrorCode
mooseKokkosInverseShellMult(Mat shell, Vec x, Vec y)
{
  void * ctx;
  LibmeshPetscCallQ(MatShellGetContext(shell, &ctx));
  LibmeshPetscCallQ(KSPSolve(static_cast<KSP>(ctx), x, y));
  return LIBMESH_PETSC_SUCCESS;
}
} // namespace

void
PMultigrid::postLinearization()
{
  // The quadrature-point Jacobian cache the levels contract holds this linearization, so this is
  // where a level's operator can be checked against itself and where every level's operator is
  // updated for it
  verifyLevelOperators();

  updateLevelOperators();

  verifyLevelMatrices();

  verifyEntityBlocks();
}

void
PMultigrid::postJacobianAssembly()
{
  // These checks apply the solver system's own operator, which manages the tags it needs itself, so
  // they run once the Jacobian sweep has released the tags it held: the state here is the state an
  // operator application sees when the solve asks for one
  verifyLevelGalerkin();

  verifyOperatorSymmetry();

  verifyCycleSymmetry();

  verifyOperatorConditioning();
}

void
PMultigrid::verifyLevelOperators()
{
  if (!_verify_level_operators)
    return;

  TIME_SECTION("verifyLevelOperators", 1);

  const auto & cache = _fe_problem.getKokkosFESystems()[_nl.number()].qpJacobianCache();

  for (const auto & level : _levels)
  {
    auto & level_sys = level->system();
    const auto & dof_map = level_sys.get_dof_map();

    auto diagonal = level_sys.solution->zero_clone();
    auto unit = level_sys.solution->zero_clone();
    auto column = level_sys.solution->zero_clone();

    level->diagonal(cache, *diagonal);

    Real error = 0;
    Real scale = 0;

    for (const auto dof : make_range(level_sys.n_dofs()))
    {
      const auto owned = dof >= dof_map.first_dof() && dof < dof_map.end_dof();

      unit->zero();

      if (owned)
        unit->set(dof, 1.0);

      unit->close();

      level->apply(cache, *unit, *column);

      // Applying the operator to the unit vector of a degree of freedom produces that column of the
      // operator, whose entry for the same degree of freedom is the diagonal entry
      if (owned)
      {
        error = std::max(error, std::abs((*column)(dof) - (*diagonal)(dof)));
        scale = std::max(scale, std::abs((*diagonal)(dof)));
      }
    }

    comm().max(error);
    comm().max(scale);

    // The two paths contract the same cache against the same basis tables in a different loop
    // order, so they agree to the roundoff of that contraction; the margin over the unit roundoff
    // of double precision covers the element and quadrature-point sums
    const auto tolerance = 1e-10 * std::max(scale, 1.0);

    _console << "p-multigrid level p = " << level->order()
             << ": operator agrees with its diagonal to " << error << " (tolerance " << tolerance
             << ")" << std::endl;

    if (error > tolerance)
      mooseError("The operator of the p-multigrid level of order ",
                 level->order(),
                 " disagrees with the diagonal the level computes by ",
                 error,
                 ", which exceeds a tolerance of ",
                 tolerance,
                 ".");
  }
}

void
PMultigrid::updateLevelOperators()
{
  TIME_SECTION("updateLevelOperators", 1);

  const auto & cache = _fe_problem.getKokkosFESystems()[_nl.number()].qpJacobianCache();

  for (const auto & level : _levels)
    level->updateOperator(cache);
}

void
PMultigrid::verifyEntityBlocks()
{
  if (!_verify_entity_blocks)
    return;

  TIME_SECTION("verifyEntityBlocks", 1);

  _console << "p-multigrid entity blocks for system '" << _nl.name() << "':\n";

  for (const auto & level : _levels)
    if (level->assembles())
      level->verifyEntityBlocks(_console);

  _console << std::endl;
}

void
PMultigrid::verifyLevelMatrices()
{
  if (!_verify_level_matrices)
    return;

  TIME_SECTION("verifyLevelMatrices", 1);

  const auto & cache = _fe_problem.getKokkosFESystems()[_nl.number()].qpJacobianCache();

  for (const auto & level : _levels)
  {
    if (!level->assembles())
      continue;

    auto & level_sys = level->system();

    auto x = level_sys.solution->zero_clone();
    auto matrix_free = level_sys.solution->zero_clone();
    auto assembled = level_sys.solution->zero_clone();

    Moose::Kokkos::LevelTransfer::fillVerificationPattern(*x, level_sys);

    // Both operators eliminate a fixed row and column, but the assembled one carries a literal
    // identity on the fixed row where the matrix-free one contributes nothing, so the two agree
    // only on a vector that is already zero at every fixed DOF -- which is what the hierarchy ever
    // applies either operator to during a real cycle
    level->zeroConstrained(*x);

    level->apply(cache, *x, *matrix_free);

    level->matrix().vector_mult(*assembled, *x);

    assembled->add(-1.0, *matrix_free);

    const auto error = assembled->linfty_norm();
    const auto scale = matrix_free->linfty_norm();

    // The two paths contract the same cache against the same basis tables in a different loop
    // order, so they agree to the roundoff of that contraction; the margin over the unit roundoff
    // of double precision covers the element and quadrature-point sums
    const auto tolerance = 1e-10 * std::max(scale, 1.0);

    _console << "p-multigrid level p = " << level->order()
             << ": assembled operator agrees with the matrix-free operator to " << error
             << " (tolerance " << tolerance << ")" << std::endl;

    // The Kokkos matrix-free constraint enforcement forms the level's operator as C^T A C plus an
    // identity on every constrained row (no asymmetric row-replacement term), so this measures, it
    // does not assume, that the assembled matrix a coarse solver factors is actually symmetric
    PetscBool is_symmetric;
    LibmeshPetscCallA(
        comm().get(),
        MatIsSymmetric(libMesh::cast_ref<libMesh::PetscMatrix<Number> &>(level->matrix()).mat(),
                       1e-10,
                       &is_symmetric));

    _console << "p-multigrid level p = " << level->order() << ": assembled operator is "
             << (is_symmetric ? "symmetric" : "NOT symmetric") << " (measured, tolerance 1e-10)"
             << std::endl;

    if (!is_symmetric)
      mooseError("The assembled operator of the p-multigrid level of order ",
                 level->order(),
                 " is not symmetric to within a tolerance of 1e-10.");

    if (error > tolerance)
      mooseError("The assembled operator of the p-multigrid level of order ",
                 level->order(),
                 " disagrees with the operator the level applies without a matrix by ",
                 error,
                 ", which exceeds a tolerance of ",
                 tolerance,
                 ".");
  }
}

void
PMultigrid::verifyLevelGalerkin()
{
  if (!_verify_level_galerkin)
    return;

  TIME_SECTION("verifyLevelGalerkin", 1);

  const auto & cache = _fe_problem.getKokkosFESystems()[_nl.number()].qpJacobianCache();
  const auto & levels = _levels;

  for (const auto i : index_range(levels))
  {
    auto & level = *levels[i];

    // The finer side of this level's transfer: the next coarse level where there is one, and this
    // system at the finest pair
    auto * const finer = i + 1 < levels.size() ? levels[i + 1].get() : nullptr;
    auto & finer_sys = finer ? finer->system() : _nl.system();

    auto x = level.system().solution->zero_clone();
    auto direct = level.system().solution->zero_clone();
    auto galerkin = level.system().solution->zero_clone();
    auto prolonged = finer_sys.solution->zero_clone();
    auto applied = finer_sys.solution->zero_clone();

    Moose::Kokkos::LevelTransfer::fillVerificationPattern(*x, level.system());

    // The hierarchy carries only corrections that vanish on the DOFs a level holds fixed, and the
    // transfers leave those DOFs out of both directions, so the comparison is made on a vector of
    // that space.
    level.zeroConstrained(*x);

    level.apply(cache, *x, *direct);

    level.prolong(*x, *prolonged);

    if (finer)
      finer->apply(cache, *prolonged, *applied);
    else
      _nl.computeKokkosJacobianVectorProduct(
          libMesh::cast_ptr<libMesh::PetscVector<Number> *>(prolonged.get())->vec(),
          libMesh::cast_ptr<libMesh::PetscVector<Number> *>(applied.get())->vec());

    level.restrictTo(*applied, *galerkin);

    galerkin->add(-1.0, *direct);

    const auto error = galerkin->linfty_norm();
    const auto scale = direct->linfty_norm();

    // Both sides contract the same quadrature-point cache against bases that span the same space,
    // in a different order and over a different number of degrees of freedom, so they agree to the
    // roundoff of those contractions; the margin over the unit roundoff of double precision covers
    // the element and quadrature-point sums
    const auto tolerance = 1e-10 * std::max(scale, 1.0);

    _console << "p-multigrid level p = " << level.order()
             << ": operator agrees with the operator of the next finer level carried through the "
                "transfer to "
             << error << " (tolerance " << tolerance << ")" << std::endl;

    if (error > tolerance)
      mooseError("The operator of the p-multigrid level of order ",
                 level.order(),
                 " differs from P^T A P, the operator of the next finer level carried through the "
                 "level transfer, by ",
                 error,
                 ", which exceeds a tolerance of ",
                 tolerance,
                 ". Every level contracts the quadrature-point Jacobian cache, so a Jacobian "
                 "contribution made outside that cache reaches the fine operator and no coarse "
                 "level, and surfaces here.");
  }
}

void
PMultigrid::verifyOperatorConditioning()
{
  if (!_verify_operator_conditioning)
    return;

  TIME_SECTION("verifyOperatorConditioning", 1);

  auto & operator_shell =
      libMesh::cast_ref<libMesh::PetscMatrix<Number> &>(_nl.kokkosMatrixFreeOperator());

  // One application per column
  Mat assembled;
  LibmeshPetscCallA(comm().get(), MatComputeOperator(operator_shell.mat(), MATAIJ, &assembled));

  // The operator is symmetric, so it factors by Cholesky. A factorization that fails is itself the
  // answer, so it is reported rather than treated as an error.
  KSP inverse;
  LibmeshPetscCallA(comm().get(), KSPCreate(comm().get(), &inverse));
  LibmeshPetscCallA(comm().get(), KSPSetOperators(inverse, assembled, assembled));
  LibmeshPetscCallA(comm().get(), KSPSetType(inverse, KSPPREONLY));
  PC inverse_pc;
  LibmeshPetscCallA(comm().get(), KSPGetPC(inverse, &inverse_pc));
  LibmeshPetscCallA(comm().get(), PCSetType(inverse_pc, PCCHOLESKY));
  LibmeshPetscCallA(comm().get(), KSPSetErrorIfNotConverged(inverse, PETSC_FALSE));

  PetscErrorCode factored = KSPSetUp(inverse);

  PetscReal operator_norm, inverse_norm = -1;

  LibmeshPetscCallA(comm().get(), MatNorm(assembled, NORM_INFINITY, &operator_norm));

  // The spread of the diagonal locates where the conditioning comes from: the norm above is a row
  // sum, dominated by whichever mode carries the largest entries, while the smallest diagonal
  // bounds the smallest eigenvalue and so bounds the norm of the inverse from below
  Vec diagonal;
  LibmeshPetscCallA(comm().get(), MatCreateVecs(assembled, nullptr, &diagonal));
  LibmeshPetscCallA(comm().get(), MatGetDiagonal(assembled, diagonal));

  PetscReal min_diagonal, max_diagonal;
  PetscInt where;
  LibmeshPetscCallA(comm().get(), VecMin(diagonal, &where, &min_diagonal));
  LibmeshPetscCallA(comm().get(), VecMax(diagonal, &where, &max_diagonal));
  LibmeshPetscCallA(comm().get(), VecDestroy(&diagonal));

  if (!factored)
  {
    PetscInt local_rows, local_cols, rows, cols;
    LibmeshPetscCallA(comm().get(), MatGetLocalSize(assembled, &local_rows, &local_cols));
    LibmeshPetscCallA(comm().get(), MatGetSize(assembled, &rows, &cols));

    // The inverse is formed column by column exactly as the cycle is, so its norm is taken over the
    // same quantity and the two are comparable
    Mat inverse_shell;
    LibmeshPetscCallA(
        comm().get(),
        MatCreateShell(comm().get(), local_rows, local_cols, rows, cols, inverse, &inverse_shell));
    LibmeshPetscCallA(comm().get(),
                      MatShellSetOperation(
                          inverse_shell, MATOP_MULT, (void (*)(void))mooseKokkosInverseShellMult));

    Mat formed_inverse;
    LibmeshPetscCallA(comm().get(), MatComputeOperator(inverse_shell, MATAIJ, &formed_inverse));
    LibmeshPetscCallA(comm().get(), MatNorm(formed_inverse, NORM_INFINITY, &inverse_norm));

    LibmeshPetscCallA(comm().get(), MatDestroy(&formed_inverse));
    LibmeshPetscCallA(comm().get(), MatDestroy(&inverse_shell));
  }

  LibmeshPetscCallA(comm().get(), KSPDestroy(&inverse));
  LibmeshPetscCallA(comm().get(), MatDestroy(&assembled));

  _console << "Kokkos matrix-free operator of system '" << _nl.name() << "': norm "
           << static_cast<Real>(operator_norm) << ", diagonal from "
           << static_cast<Real>(min_diagonal) << " to " << static_cast<Real>(max_diagonal);

  if (inverse_norm < 0)
    _console << ", inverse could not be factored by Cholesky, so the operator is numerically "
                "singular at this order"
             << std::endl;
  else
    _console << ", inverse norm " << static_cast<Real>(inverse_norm) << ", product "
             << static_cast<Real>(operator_norm) * static_cast<Real>(inverse_norm) << std::endl;
}

void
PMultigrid::verifyCycleSymmetry()
{
  if (!_verify_cycle_symmetry)
    return;

  TIME_SECTION("verifyCycleSymmetry", 1);

  KSP ksp;
  LibmeshPetscCallA(comm().get(), SNESGetKSP(_nl.getSNES(), &ksp));
  PC pc;
  LibmeshPetscCallA(comm().get(), KSPGetPC(ksp, &pc));

  // A preconditioner has to be built before it can be applied. The levels' operators are up to date
  // by this point, so what is measured is the cycle the solve about to run will apply.
  LibmeshPetscCallA(comm().get(), PCSetUp(pc));

  // The cycle is a linear operator over the same layout the system's own operator is over
  auto & operator_shell =
      libMesh::cast_ref<libMesh::PetscMatrix<Number> &>(_nl.kokkosMatrixFreeOperator());

  PetscInt local_rows, local_cols, rows, cols;
  LibmeshPetscCallA(comm().get(), MatGetLocalSize(operator_shell.mat(), &local_rows, &local_cols));
  LibmeshPetscCallA(comm().get(), MatGetSize(operator_shell.mat(), &rows, &cols));

  Mat cycle;
  LibmeshPetscCallA(comm().get(),
                    MatCreateShell(comm().get(), local_rows, local_cols, rows, cols, pc, &cycle));
  LibmeshPetscCallA(
      comm().get(),
      MatShellSetOperation(cycle, MATOP_MULT, (void (*)(void))mooseKokkosPreconditionerShellMult));

  // One application of the whole cycle per column, which is why this is gated behind a flag
  Mat assembled;
  LibmeshPetscCallA(comm().get(), MatComputeOperator(cycle, MATAIJ, &assembled));

  Mat asymmetry;
  LibmeshPetscCallA(comm().get(), MatTranspose(assembled, MAT_INITIAL_MATRIX, &asymmetry));
  LibmeshPetscCallA(comm().get(), MatAXPY(asymmetry, -1.0, assembled, DIFFERENT_NONZERO_PATTERN));

  PetscReal asymmetry_norm, cycle_norm;
  LibmeshPetscCallA(comm().get(), MatNorm(asymmetry, NORM_INFINITY, &asymmetry_norm));
  LibmeshPetscCallA(comm().get(), MatNorm(assembled, NORM_INFINITY, &cycle_norm));

  LibmeshPetscCallA(comm().get(), MatDestroy(&asymmetry));
  LibmeshPetscCallA(comm().get(), MatDestroy(&assembled));
  LibmeshPetscCallA(comm().get(), MatDestroy(&cycle));

  const auto error = static_cast<Real>(asymmetry_norm);
  const auto scale = static_cast<Real>(cycle_norm);

  // A v-cycle is symmetric when each of its parts is: the smoothers are symmetric, the coarse solve
  // is a symmetric factorization, and restriction is the transpose of prolongation. What remains is
  // the roundoff of the sweeps, transfers and triangular solves the cycle performs, which scales
  // with the norm of the cycle itself, so the margin is relative to that norm and matches the one
  // the operator's own symmetry is held to.
  const auto tolerance = 1e-10 * std::max(scale, 1.0);

  _console << "Kokkos p-multigrid cycle of system '" << _nl.name()
           << "': differs from its transpose by " << error << " (tolerance " << tolerance
           << ", cycle norm " << scale << ")" << std::endl;

  if (error > tolerance)
    mooseError("The p-multigrid cycle of system '",
               _nl.name(),
               "' differs from its transpose by ",
               error,
               ", which exceeds a tolerance of ",
               tolerance,
               ". A Krylov method that requires a symmetric preconditioner, such as CG, is not "
               "valid over it. An asymmetric smoother, a coarse level solved by an iteration "
               "rather than a factorization, or a restriction that is not the transpose of the "
               "prolongation are the causes to check.");
}

void
PMultigrid::verifyOperatorSymmetry()
{
  if (!_verify_operator_symmetry)
    return;

  TIME_SECTION("verifyOperatorSymmetry", 1);

  const auto & fe_system = _fe_problem.getKokkosFESystems()[_nl.number()];

  // Refuse to measure what the operator is not built to satisfy, rather than reporting the
  // asymmetry a transient keeps by design as a tolerance failure
  if (!fe_system.eliminatesConstrainedColumns() && fe_system.numConstrainedDofs())
    mooseError(
        "Verification of the symmetry of the Kokkos matrix-free operator was requested for system "
        "'",
        _nl.name(),
        "', which is transient and has Dirichlet-constrained degrees of freedom. A transient's "
        "time integrator forms the solution time derivative from the raw solution, so a free row's "
        "residual reads a constrained degree of freedom's own iterate value, and the operator has "
        "to keep the matching column to remain the linearization of that residual. The operator is "
        "therefore asymmetric by design here, and a Krylov method requiring symmetry, such as CG, "
        "is not available for a transient Kokkos matrix-free system.");

  // The shell registered as this system's own matrix, which PetscNonlinearSolver hands SNES as both
  // Amat and Pmat, and whose MatMult is _nl.computeKokkosJacobianVectorProduct()
  auto & shell = libMesh::cast_ref<libMesh::PetscMatrix<Number> &>(_nl.kokkosMatrixFreeOperator());

  // One application per column, which is why this is gated behind a verification flag
  Mat assembled;
  LibmeshPetscCallA(comm().get(), MatComputeOperator(shell.mat(), MATAIJ, &assembled));

  Mat asymmetry;
  LibmeshPetscCallA(comm().get(), MatTranspose(assembled, MAT_INITIAL_MATRIX, &asymmetry));
  LibmeshPetscCallA(comm().get(), MatAXPY(asymmetry, -1.0, assembled, DIFFERENT_NONZERO_PATTERN));

  PetscReal asymmetry_norm, operator_norm;
  LibmeshPetscCallA(comm().get(), MatNorm(asymmetry, NORM_INFINITY, &asymmetry_norm));
  LibmeshPetscCallA(comm().get(), MatNorm(assembled, NORM_INFINITY, &operator_norm));

  LibmeshPetscCallA(comm().get(), MatDestroy(&asymmetry));
  LibmeshPetscCallA(comm().get(), MatDestroy(&assembled));

  const auto error = static_cast<Real>(asymmetry_norm);
  const auto scale = static_cast<Real>(operator_norm);

  // The two sides of the transpose are the same quadrature-point contraction with the test and
  // trial basis functions exchanged, so they agree to the roundoff of that contraction; the margin
  // over the unit roundoff of double precision covers the element and quadrature-point sums
  const auto tolerance = 1e-10 * std::max(scale, 1.0);

  _console << "Kokkos matrix-free operator of system '" << _nl.name()
           << "': differs from its transpose by " << error << " (tolerance " << tolerance << ")"
           << std::endl;

  if (error > tolerance)
    mooseError(
        "The Kokkos matrix-free Jacobian action of system '",
        _nl.name(),
        "' differs from its transpose by ",
        error,
        ", which exceeds a tolerance of ",
        tolerance,
        ". A Krylov method that requires a symmetric operator, such as CG, is not valid on "
        "it. A Dirichlet-constrained degree of freedom left in a free row's residual or in "
        "the operator's trial-space gather is the usual cause, since the constrained row's "
        "own action is the identity and so has nothing to match in the transposed position.");
}

#endif
