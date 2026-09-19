//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosQpJacobianOperator.h"
#include "KokkosUtils.h"

#include "libmesh/petsc_solver_exception.h"

namespace Moose::Kokkos
{

QpJacobianOperator::QpJacobianOperator(const Assembly & assembly,
                                       const QpJacobianCache & cache,
                                       const QpJacobianLevel & level)
  : AssemblyHolder(assembly), _cache(cache), _level(level)
{
  // Every level lives on the mesh the fine level does, so the loops of every level run over the
  // same (element, variable) pairs
  _thread.resize(_level.kokkosMesh().getNumLocalElements(), _level.getNumVars());
}

void
QpJacobianOperator::apply(const TagID x_tag, const TagID y_tag)
{
  _x_tag = x_tag;
  _y_tag = y_tag;

  if (_apply_flops < 0)
  {
    ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, ApplyFlopLoop> flop_policy(
        0, _thread.size());
    double flops = 0;
    ::Kokkos::parallel_reduce(flop_policy, *this, flops);
    ::Kokkos::fence();
    _apply_flops = flops;
  }

  if (!_apply_scratch_bytes)
  {
    ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, ApplyScratchLoop> scratch_policy(
        0, _thread.size());
    std::size_t bytes = 0;
    ::Kokkos::parallel_reduce(
        scratch_policy, *this, ::Kokkos::Max<std::size_t>(bytes));
    ::Kokkos::fence();
    _apply_scratch_bytes = bytes;
  }

  ApplyPolicy policy(_thread.size(), ::Kokkos::AUTO);
  policy.set_scratch_size(0, ::Kokkos::PerTeam(_apply_scratch_bytes));
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();

  // Reported so that a shell operator's applications carry a flop rate in PETSc's log alongside
  // the events of the assembled operators they are compared against
  LibmeshPetscCallA(PETSC_COMM_SELF, PetscLogGpuFlops(_apply_flops));
}

/**
 * The operations the team loop performs on one (element, variable) pair. Both contractions run over
 * the fixed three components of the reference gradient whatever the mesh dimension is, because the
 * cached tensor's gradient blocks are Real3 and Real33, so the counts below are dimension
 * independent and match what the hardware issues.
 */
namespace
{
/// applyB(): per DOF, one multiply-add for the value and three for the Real3 reference gradient
constexpr double APPLY_GATHER_FLOPS_PER_DOF = 8;
/// applyBT(): per DOF, one multiply for the value and a Real3 dot for the gradient, accumulated
constexpr double APPLY_SCATTER_FLOPS_PER_DOF = 8;
/// The value/flux pair: vv * value + vg . grad, then gv * value + gg * grad
constexpr double APPLY_TENSOR_FLOPS_PER_QP = 28;
} // namespace

namespace
{
/// Scratch views the team loop carves out of its per-team allocation
///@{
using ScratchReal =
    ::Kokkos::View<Real *, ExecSpace::scratch_memory_space, ::Kokkos::MemoryUnmanaged>;
using ScratchReal3 =
    ::Kokkos::View<Real3 *, ExecSpace::scratch_memory_space, ::Kokkos::MemoryUnmanaged>;
///@}

/**
 * The per-team scratch bytes the team loop carves out for an element, allowing for the alignment
 * each separate scratch allocation is padded to
 */
KOKKOS_INLINE_FUNCTION std::size_t
applyScratchBytes(const unsigned int num_dofs, const unsigned int num_qps)
{
  constexpr std::size_t alignment = 128;

  return num_dofs * sizeof(Real) + num_qps * sizeof(Real) + num_qps * sizeof(Real3) +
         3 * alignment;
}
} // namespace

KOKKOS_FUNCTION void
QpJacobianOperator::operator()(ApplyScratchLoop, const ThreadID tid, std::size_t & bytes) const
{
  auto elem = _thread(tid, 0);
  auto var = _thread(tid, 1);

  auto info = _level.kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;

  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  const LevelBasisTable basis(
      kokkosAssembly(), sid, info.type, _level.getFEType(var), info.orientation);

  const auto needed = applyScratchBytes(basis.numDofs(), kokkosAssembly().getNumQps(info));

  if (needed > bytes)
    bytes = needed;
}

/**
 * One team per (element, variable) pair. The trial-space gather is done once into scratch and the
 * quadrature-point contraction is shared across the team, where a thread-per-element loop repeats
 * the gather for every batch of test functions its accumulator can hold. Each test function is
 * summed over the quadrature points in index order and each gather runs over the DOFs in index
 * order, the same orders a thread-per-element loop uses, so the result is unchanged.
 */
KOKKOS_FUNCTION void
QpJacobianOperator::operator()(ApplyTeamLoop, const ApplyTeam & team) const
{
  const ThreadID tid = team.league_rank();

  auto elem = _thread(tid, 0);
  auto var = _thread(tid, 1);

  auto info = _level.kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;

  // A pair with no declared blocks has no cached linearization; the objects responsible for it
  // supply their own operator action. The test is the same for every thread of the team.
  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  auto num_qps = kokkosAssembly().getNumQps(info);
  auto qp_offset = kokkosAssembly().getQpOffset(info);

  const LevelBasisTable basis(
      kokkosAssembly(), sid, info.type, _level.getFEType(var), info.orientation);

  const auto num_dofs = basis.numDofs();

  ScratchReal x_e(team.team_scratch(0), num_dofs);
  ScratchReal f_qp(team.team_scratch(0), num_qps);
  ScratchReal3 flux_qp(team.team_scratch(0), num_qps);

  // Gather the direction into the element's trial space once for the whole team. Where the level
  // eliminates a fixed column, that row's direction is zero.
  ::Kokkos::parallel_for(::Kokkos::TeamThreadRange(team, num_dofs),
                         [&](const unsigned int j)
                         {
                           const auto dof = _level.getElemLocalDofIndex(elem, j, var);

                           x_e(j) = _level.isEliminatedColumn(dof)
                                        ? 0.0
                                        : _level.getDofValue(dof, _x_tag);
                         });

  team.team_barrier();

  // Contract the linearization at each quadrature point into the value/flux pair the test
  // functions are contracted against
  ::Kokkos::parallel_for(
      ::Kokkos::TeamThreadRange(team, num_qps),
      [&](const unsigned int qp)
      {
        Real value = 0;
        Real3 grad = 0;

        for (unsigned int j = 0; j < num_dofs; ++j)
        {
          Real phi;
          Real3 grad_phi;
          basis.basis(j, qp, phi, grad_phi);

          const auto x_j = x_e(j);

          value += x_j * phi;
          grad += x_j * grad_phi;
        }

        const auto & tensor = _cache.getTensor(sid, var, qp_offset + qp);

        f_qp(qp) = tensor.vv * value + tensor.vg * grad;
        flux_qp(qp) = tensor.gv * value + tensor.gg * grad;
      });

  team.team_barrier();

  // Accumulate each test function over the quadrature points and reduce it into its row
  ::Kokkos::parallel_for(::Kokkos::TeamThreadRange(team, num_dofs),
                         [&](const unsigned int i)
                         {
                           auto row = _level.getElemLocalDofIndex(elem, i, var);

                           if (_level.isConstrained(row))
                             return;

                           Real y_i = 0;

                           for (unsigned int qp = 0; qp < num_qps; ++qp)
                           {
                             Real phi;
                             Real3 grad_phi;
                             basis.basis(i, qp, phi, grad_phi);

                             y_i += phi * f_qp(qp) + grad_phi * flux_qp(qp);
                           }

                           ::Kokkos::atomic_add(&_level.getDofValue(row, _y_tag), y_i);
                         });
}

KOKKOS_FUNCTION void
QpJacobianOperator::operator()(ApplyFlopLoop, const ThreadID tid, double & flops) const
{
  auto elem = _thread(tid, 0);
  auto var = _thread(tid, 1);

  auto info = _level.kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;

  // A pair the team loop returns early on contributes nothing
  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  const auto num_qps = kokkosAssembly().getNumQps(info);

  const LevelBasisTable basis(
      kokkosAssembly(), sid, info.type, _level.getFEType(var), info.orientation);

  const auto num_dofs = basis.numDofs();

  flops += num_qps * (num_dofs * APPLY_GATHER_FLOPS_PER_DOF + APPLY_TENSOR_FLOPS_PER_QP +
                      num_dofs * APPLY_SCATTER_FLOPS_PER_DOF);
}

void
QpJacobianOperator::diagonal(const TagID diag_tag)
{
  _y_tag = diag_tag;

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, DiagonalLoop> policy(
      0, _thread.size());
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();
}

void
QpJacobianOperator::assemble(Matrix & matrix)
{
  _matrix = matrix;

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, MatrixLoop> policy(
      0, _thread.size());
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();
}

void
QpJacobianOperator::assembleBlocks(const EntityBlocks & blocks)
{
  _blocks = blocks;

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, BlockLoop> policy(0,
                                                                                    _thread.size());
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();
}

void
QpJacobianOperator::applyBlocks(const EntityBlocks & blocks, const TagID r_tag, const TagID x_tag)
{
  _blocks = blocks;
  _x_tag = r_tag;
  _y_tag = x_tag;

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<dof_id_type>, BlockApplyLoop> policy(
      0, blocks.numBlocks());
  ::Kokkos::parallel_for(policy, *this);

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<dof_id_type>, BlockIdentityLoop> identity(
      0, blocks.getNumLocalDofs());
  ::Kokkos::parallel_for(identity, *this);

  ::Kokkos::fence();
}


KOKKOS_FUNCTION void
QpJacobianOperator::operator()(DiagonalLoop, const ThreadID tid) const
{
  auto elem = _thread(tid, 0);
  auto var = _thread(tid, 1);

  auto info = _level.kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;

  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  auto num_qps = kokkosAssembly().getNumQps(info);
  auto qp_offset = kokkosAssembly().getQpOffset(info);

  const LevelBasisTable basis(
      kokkosAssembly(), sid, info.type, _level.getFEType(var), info.orientation);

  for (unsigned int i = 0; i < basis.numDofs(); ++i)
  {
    auto row = _level.getElemLocalDofIndex(elem, i, var);

    if (_level.isConstrained(row))
      continue;

    Real diagonal = 0;

    // The diagonal entry is the operator with the same basis function in the test and trial slot
    for (unsigned int qp = 0; qp < num_qps; ++qp)
    {
      const auto & tensor = _cache.getTensor(sid, var, qp_offset + qp);

      Real value;
      Real3 grad;

      basis.basis(i, qp, value, grad);

      diagonal += value * (tensor.vv * value + tensor.vg * grad) +
                  grad * (tensor.gv * value + tensor.gg * grad);
    }

    ::Kokkos::atomic_add(&_level.getDofValue(row, _y_tag), diagonal);
  }
}

KOKKOS_FUNCTION void
QpJacobianOperator::operator()(MatrixLoop, const ThreadID tid) const
{
  auto elem = _thread(tid, 0);
  auto var = _thread(tid, 1);

  auto info = _level.kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;

  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  auto num_qps = kokkosAssembly().getNumQps(info);
  auto qp_offset = kokkosAssembly().getQpOffset(info);

  const LevelBasisTable basis(
      kokkosAssembly(), sid, info.type, _level.getFEType(var), info.orientation);

  const auto num_dofs = basis.numDofs();

  for (unsigned int i = 0; i < num_dofs; ++i)
  {
    auto row = _level.getElemLocalDofIndex(elem, i, var);

    if (_level.isConstrained(row))
      continue;

    for (unsigned int j = 0; j < num_dofs; ++j)
    {
      auto col = _level.getElemLocalDofIndex(elem, j, var);

      // Skipping a fixed column as well as a fixed row makes the assembled matrix symmetric
      // whenever the linearization it contracts is. Only a p-multigrid level assembles a matrix,
      // and a level always eliminates its fixed columns, so this is the same elimination apply()
      // above makes in its trial-space gather and the two are one operator in assembled and
      // matrix-free form.
      if (_level.isConstrained(col))
        continue;

      Real entry = 0;

      for (unsigned int qp = 0; qp < num_qps; ++qp)
      {
        const auto & tensor = _cache.getTensor(sid, var, qp_offset + qp);

        Real value_i, value_j;
        Real3 grad_i, grad_j;

        basis.basis(i, qp, value_i, grad_i);
        basis.basis(j, qp, value_j, grad_j);

        entry += value_i * (tensor.vv * value_j + tensor.vg * grad_j) +
                 grad_i * (tensor.gv * value_j + tensor.gg * grad_j);
      }

      ::Kokkos::atomic_add(&_matrix(row, _level.getElemGlobalDofIndex(elem, j, var)), entry);
    }
  }
}

KOKKOS_FUNCTION void
QpJacobianOperator::operator()(BlockLoop, const ThreadID tid) const
{
  auto elem = _thread(tid, 0);
  auto var = _thread(tid, 1);

  auto info = _level.kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;

  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  auto num_qps = kokkosAssembly().getNumQps(info);
  auto qp_offset = kokkosAssembly().getQpOffset(info);

  const LevelBasisTable basis(
      kokkosAssembly(), sid, info.type, _level.getFEType(var), info.orientation);

  const auto num_dofs = basis.numDofs();

  // The decomposition holds no fixed row and no row this process does not own, so the pair tests
  // below are what confine the contraction to the entries of a block this process is building. An
  // entity's block gathers a contribution from every element that carries the entity, which for an
  // interior block is one element and for a shared edge or face is each element on it.
  for (unsigned int i = 0; i < num_dofs; ++i)
  {
    const auto row = _level.getElemLocalDofIndex(elem, i, var);

    if (!_blocks.isBlocked(row))
      continue;

    const auto block = _blocks.blockOfDof(row);
    const auto row_pos = _blocks.posOfDof(row);

    for (unsigned int j = 0; j < num_dofs; ++j)
    {
      const auto col = _level.getElemLocalDofIndex(elem, j, var);

      if (!_blocks.isBlocked(col) || _blocks.blockOfDof(col) != block)
        continue;

      Real entry = 0;

      for (unsigned int qp = 0; qp < num_qps; ++qp)
      {
        const auto & tensor = _cache.getTensor(sid, var, qp_offset + qp);

        Real value_i, value_j;
        Real3 grad_i, grad_j;

        basis.basis(i, qp, value_i, grad_i);
        basis.basis(j, qp, value_j, grad_j);

        entry += value_i * (tensor.vv * value_j + tensor.vg * grad_j) +
                 grad_i * (tensor.gv * value_j + tensor.gg * grad_j);
      }

      ::Kokkos::atomic_add(&_blocks.entry(block, row_pos, _blocks.posOfDof(col)), entry);
    }
  }
}

KOKKOS_FUNCTION void
QpJacobianOperator::operator()(BlockApplyLoop, const dof_id_type block) const
{
  const auto size = _blocks.blockSize(block);

  Real residual[MAX_BLOCK_DOF];
  Real correction[MAX_BLOCK_DOF];

  for (unsigned int i = 0; i < size; ++i)
    residual[i] = _level.getDofValueRead(_blocks.blockDof(block, i), _x_tag);

  if (_blocks.isFactored(block))
    Utils::choleskySubstitute(_blocks.blockEntries(block), correction, residual, size);
  else
    // A block that carries no factor is applied as its diagonal, which is the point smoother on
    // those rows. A zero diagonal leaves the row uncorrected rather than dividing by it.
    for (unsigned int i = 0; i < size; ++i)
    {
      const auto diagonal = _blocks.entry(block, i, i);

      correction[i] = diagonal ? residual[i] / diagonal : residual[i];
    }

  for (unsigned int i = 0; i < size; ++i)
    _level.getDofValue(_blocks.blockDof(block, i), _y_tag) = correction[i];
}

KOKKOS_FUNCTION void
QpJacobianOperator::operator()(BlockIdentityLoop, const dof_id_type dof) const
{
  // A row no block covers is a row the level holds fixed, and the level's operator carries the
  // identity there, so the smoother reproduces it rather than leaving the row unset
  if (!_blocks.isBlocked(dof))
    _level.getDofValue(dof, _y_tag) = _level.getDofValueRead(dof, _x_tag);
}

} // namespace Moose::Kokkos
