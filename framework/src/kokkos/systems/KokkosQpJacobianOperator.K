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

#include <algorithm>

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

    ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, ApplyTensorFlopLoop>
        tensor_flop_policy(0, _thread.size());
    double tensor_flops = 0;
    ::Kokkos::parallel_reduce(tensor_flop_policy, *this, tensor_flops);

    ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, ApplyTensorSizeLoop>
        tensor_size_policy(0, _thread.size());
    unsigned int tensor_team_size = 0;
    ::Kokkos::parallel_reduce(
        tensor_size_policy, *this, ::Kokkos::Max<unsigned int>(tensor_team_size));

    ::Kokkos::fence();
    _apply_flops = flops;
    _apply_tensor_flops = tensor_flops;
    _apply_tensor_team_size = tensor_team_size;
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

  if (_apply_flops > 0)
  {
    ApplyPolicy policy(_thread.size(), ::Kokkos::AUTO);
    policy.set_scratch_size(0, ::Kokkos::PerTeam(_apply_scratch_bytes));
    ::Kokkos::parallel_for(policy, *this);
  }

  if (_apply_tensor_flops > 0)
  {
    // The team wants a thread per index of the largest range its phases run over, capped by what the
    // functor's register and scratch use leaves room for. The device rounds the request up to its own
    // scheduling granularity, so the size is requested as measured rather than rounded here.
    ApplyTensorPolicy policy(_thread.size(), ::Kokkos::AUTO);
    policy.set_scratch_size(0, ::Kokkos::PerTeam(_apply_scratch_bytes));

    const auto max_team_size =
        static_cast<unsigned int>(policy.team_size_max(*this, ::Kokkos::ParallelForTag()));
    const auto team_size = ::std::min(_apply_tensor_team_size, max_team_size);

    ApplyTensorPolicy sized_policy(_thread.size(), team_size);
    sized_policy.set_scratch_size(0, ::Kokkos::PerTeam(_apply_scratch_bytes));
    ::Kokkos::parallel_for(sized_policy, *this);
  }

  ::Kokkos::fence();

  // Reported so that a shell operator's applications carry a flop rate in PETSc's log alongside
  // the events of the assembled operators they are compared against
  LibmeshPetscCallA(PETSC_COMM_SELF, PetscLogGpuFlops(_apply_flops + _apply_tensor_flops));
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

/**
 * The floating point operations the sum-factorized loop performs on one (element, variable) pair.
 *
 * Every term of every contraction is a multiply-add, so each is counted as two operations. Going
 * forward, contracting the first reference coordinate's modes onto its quadrature points produces an
 * interpolant and a derivative, and contracting the second's then produces the value and both
 * gradient components; coming back, the same two contractions run in the opposite order, the first
 * of them carrying the value alongside the second coordinate's flux. The gather and the scatter each
 * apply the sign of a mode pair. The quadrature-point tensor application is the one the dense loop
 * performs.
 */
KOKKOS_INLINE_FUNCTION double
applyTensorFlops(const unsigned int n_modes, const unsigned int n_points)
{
  const double modes = n_modes;
  const double points = n_points;

  const double coordinate_pair = 2 * (2 * points * modes * modes + 3 * points * points * modes);

  return 2 * coordinate_pair + 2 * modes * modes + APPLY_TENSOR_FLOPS_PER_QP * points * points;
}
} // namespace

namespace
{
/// Scratch views the team loop carves out of its per-team allocation
///@{
using ScratchReal =
    ::Kokkos::View<Real *, ExecSpace::scratch_memory_space, ::Kokkos::MemoryUnmanaged>;
using ScratchReal3 =
    ::Kokkos::View<Real3 *, ExecSpace::scratch_memory_space, ::Kokkos::MemoryUnmanaged>;
using ScratchTensor =
    ::Kokkos::View<QpJacobianTensor *, ExecSpace::scratch_memory_space, ::Kokkos::MemoryUnmanaged>;
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

/**
 * The per-team scratch bytes the sum-factorized loop carves out for an element, allowing for the
 * alignment each separate scratch allocation is padded to
 */
KOKKOS_INLINE_FUNCTION std::size_t
applyTensorScratchBytes(const unsigned int n_modes, const unsigned int n_points)
{
  constexpr std::size_t alignment = 128;

  return (n_modes * n_modes + 2 * n_points * n_modes + 3 * n_points * n_points) * sizeof(Real) +
         6 * alignment;
}

/**
 * The per-team scratch the block assembly carves out for an element, holding that element's
 * quadrature-point linearization, allowing for the alignment the allocation is padded to
 */
KOKKOS_INLINE_FUNCTION std::size_t
blockScratchBytes(const unsigned int num_qps)
{
  constexpr std::size_t alignment = 128;

  return num_qps * sizeof(QpJacobianTensor) + alignment;
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

  const auto fe_type = _level.getFEType(var);
  const auto n_modes = kokkosAssembly().getNumTensorModes(sid, info.type, fe_type);

  const LevelBasisTable basis(kokkosAssembly(), sid, info.type, fe_type, info.orientation);

  // Whichever loop owns the pair is the one whose scratch it asks for
  const auto needed =
      n_modes
          ? applyTensorScratchBytes(n_modes,
                                    kokkosAssembly().getTensorPhi(sid, info.type, fe_type).n(1))
          : applyScratchBytes(basis.numDofs(), kokkosAssembly().getNumQps(info));

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

  // A pair whose basis factors is the sum-factorized loop's
  if (kokkosAssembly().getNumTensorModes(sid, info.type, _level.getFEType(var)))
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

  // A pair either team loop returns early on contributes nothing to that loop
  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  if (kokkosAssembly().getNumTensorModes(sid, info.type, _level.getFEType(var)))
    return;

  const auto num_qps = kokkosAssembly().getNumQps(info);

  const LevelBasisTable basis(
      kokkosAssembly(), sid, info.type, _level.getFEType(var), info.orientation);

  const auto num_dofs = basis.numDofs();

  flops += num_qps * (num_dofs * APPLY_GATHER_FLOPS_PER_DOF + APPLY_TENSOR_FLOPS_PER_QP +
                      num_dofs * APPLY_SCATTER_FLOPS_PER_DOF);
}

/**
 * One team per (element, variable) pair whose basis factors into a product of one-dimensional shape
 * functions. The contraction runs one reference coordinate at a time, against the one-dimensional
 * tables, in place of the dense contraction against the two-dimensional ones.
 *
 * Summing a coordinate at a time changes the order the quadrature points and the modes are
 * accumulated in, so the action differs from the dense loop's in its last digits. It is the same
 * operator.
 */
KOKKOS_FUNCTION void
QpJacobianOperator::operator()(ApplyTensorLoop, const ApplyTensorTeam & team) const
{
  const ThreadID tid = team.league_rank();

  auto elem = _thread(tid, 0);
  auto var = _thread(tid, 1);

  auto info = _level.kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;

  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  const auto fe_type = _level.getFEType(var);
  const auto n_modes = kokkosAssembly().getNumTensorModes(sid, info.type, fe_type);

  // A pair whose basis does not factor is the dense loop's
  if (!n_modes)
    return;

  const auto & phi_1d = kokkosAssembly().getTensorPhi(sid, info.type, fe_type);
  const auto & grad_phi_1d = kokkosAssembly().getTensorGradPhi(sid, info.type, fe_type);
  const auto & modes = kokkosAssembly().getTensorModes(sid, info.type, fe_type, info.orientation);

  const unsigned int n_points = phi_1d.n(1);
  const auto qp_offset = kokkosAssembly().getQpOffset(info);

  // The direction in tensor order, the partial contraction over one reference coordinate, and the
  // quadrature-point value/flux pair. The partial contraction is filled going forward and again
  // coming back, which is why it carries no more descriptive a name.
  ScratchReal x_e(team.team_scratch(0), n_modes * n_modes);
  ScratchReal t_value(team.team_scratch(0), n_points * n_modes);
  ScratchReal t_deriv(team.team_scratch(0), n_points * n_modes);
  ScratchReal f_qp(team.team_scratch(0), n_points * n_points);
  ScratchReal flux_xi(team.team_scratch(0), n_points * n_points);
  ScratchReal flux_eta(team.team_scratch(0), n_points * n_points);

  // Gather the direction into the element's trial space in tensor order, with the sign the basis
  // puts on a mode pair folded in. Where the level eliminates a fixed column, that row's direction
  // is zero.
  ::Kokkos::parallel_for(::Kokkos::TeamThreadRange(team, n_modes * n_modes),
                         [&](const unsigned int m)
                         {
                           const auto & mode = modes(m % n_modes, m / n_modes);
                           const auto dof = _level.getElemLocalDofIndex(elem, mode.dof, var);

                           x_e(m) = _level.isEliminatedColumn(dof)
                                        ? 0.0
                                        : mode.sign * _level.getDofValue(dof, _x_tag);
                         });

  team.team_barrier();

  // Contract the first coordinate's modes onto its quadrature points, which along each line of the
  // second coordinate's modes leaves the interpolant and its first-coordinate derivative
  ::Kokkos::parallel_for(::Kokkos::TeamThreadRange(team, n_points * n_modes),
                         [&](const unsigned int line)
                         {
                           const auto p0 = line % n_points;
                           const auto i1 = line / n_points;

                           Real value = 0;
                           Real deriv = 0;

                           for (unsigned int i0 = 0; i0 < n_modes; ++i0)
                           {
                             const auto x_j = x_e(i0 + i1 * n_modes);

                             value += phi_1d(i0, p0) * x_j;
                             deriv += grad_phi_1d(i0, p0) * x_j;
                           }

                           t_value(line) = value;
                           t_deriv(line) = deriv;
                         });

  team.team_barrier();

  // Contract the second coordinate's modes onto its quadrature points, completing the value and
  // reference gradient, and apply the linearization there. The third gradient component is zero on
  // a two-dimensional element, as it is in the tables the dense loop reads, so the flux component
  // that pairs with it is dropped.
  ::Kokkos::parallel_for(
      ::Kokkos::TeamThreadRange(team, n_points * n_points),
      [&](const unsigned int qp)
      {
        const auto p0 = qp % n_points;
        const auto p1 = qp / n_points;

        Real value = 0;
        Real3 grad = 0;

        for (unsigned int i1 = 0; i1 < n_modes; ++i1)
        {
          const auto line = p0 + i1 * n_points;
          const auto phi = phi_1d(i1, p1);

          value += phi * t_value(line);
          grad(0) += phi * t_deriv(line);
          grad(1) += grad_phi_1d(i1, p1) * t_value(line);
        }

        const auto & tensor = _cache.getTensor(sid, var, qp_offset + qp);
        const Real3 flux = tensor.gv * value + tensor.gg * grad;

        f_qp(qp) = tensor.vv * value + tensor.vg * grad;
        flux_xi(qp) = flux(0);
        flux_eta(qp) = flux(1);
      });

  team.team_barrier();

  // Contract the second coordinate's quadrature points back onto its modes. A test function's value
  // and its second-coordinate derivative share the same first-coordinate factor, so the value and
  // the second-coordinate flux accumulate together, while the first-coordinate flux pairs with the
  // first-coordinate derivative and accumulates on its own.
  ::Kokkos::parallel_for(::Kokkos::TeamThreadRange(team, n_points * n_modes),
                         [&](const unsigned int line)
                         {
                           const auto p0 = line % n_points;
                           const auto i1 = line / n_points;

                           Real value = 0;
                           Real deriv = 0;

                           for (unsigned int p1 = 0; p1 < n_points; ++p1)
                           {
                             const auto qp = p0 + p1 * n_points;
                             const auto phi = phi_1d(i1, p1);

                             value += phi * f_qp(qp) + grad_phi_1d(i1, p1) * flux_eta(qp);
                             deriv += phi * flux_xi(qp);
                           }

                           t_value(line) = value;
                           t_deriv(line) = deriv;
                         });

  team.team_barrier();

  // Contract the first coordinate's quadrature points back onto its modes and reduce each test
  // function into its row
  ::Kokkos::parallel_for(::Kokkos::TeamThreadRange(team, n_modes * n_modes),
                         [&](const unsigned int m)
                         {
                           const auto i0 = m % n_modes;
                           const auto i1 = m / n_modes;

                           const auto & mode = modes(i0, i1);
                           const auto row = _level.getElemLocalDofIndex(elem, mode.dof, var);

                           if (_level.isConstrained(row))
                             return;

                           Real y_i = 0;

                           for (unsigned int p0 = 0; p0 < n_points; ++p0)
                           {
                             const auto line = p0 + i1 * n_points;
                             y_i += phi_1d(i0, p0) * t_value(line) +
                                    grad_phi_1d(i0, p0) * t_deriv(line);
                           }

                           ::Kokkos::atomic_add(&_level.getDofValue(row, _y_tag), mode.sign * y_i);
                         });
}

KOKKOS_FUNCTION void
QpJacobianOperator::operator()(ApplyTensorFlopLoop, const ThreadID tid, double & flops) const
{
  auto elem = _thread(tid, 0);
  auto var = _thread(tid, 1);

  auto info = _level.kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;

  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  const auto fe_type = _level.getFEType(var);
  const auto n_modes = kokkosAssembly().getNumTensorModes(sid, info.type, fe_type);

  if (!n_modes)
    return;

  flops += applyTensorFlops(n_modes, kokkosAssembly().getTensorPhi(sid, info.type, fe_type).n(1));
}

KOKKOS_FUNCTION void
QpJacobianOperator::operator()(ApplyTensorSizeLoop,
                              const ThreadID tid,
                              unsigned int & team_size) const
{
  auto elem = _thread(tid, 0);
  auto var = _thread(tid, 1);

  auto info = _level.kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;

  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  const auto fe_type = _level.getFEType(var);
  const auto n_modes = kokkosAssembly().getNumTensorModes(sid, info.type, fe_type);

  if (!n_modes)
    return;

  const unsigned int n_points = kokkosAssembly().getTensorPhi(sid, info.type, fe_type).n(1);

  // Every phase runs over one of n_modes * n_modes, n_points * n_modes or n_points * n_points, so
  // the square of the larger count covers all three
  const auto side = n_modes > n_points ? n_modes : n_points;

  if (side * side > team_size)
    team_size = side * side;
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

  if (!_block_scratch_bytes)
  {
    ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, BlockScratchLoop> scratch_policy(
        0, _thread.size());
    std::size_t bytes = 0;
    ::Kokkos::parallel_reduce(scratch_policy, *this, ::Kokkos::Max<std::size_t>(bytes));
    ::Kokkos::fence();
    _block_scratch_bytes = bytes;
  }

  BlockPolicy policy(_thread.size(), ::Kokkos::AUTO);
  policy.set_scratch_size(0, ::Kokkos::PerTeam(_block_scratch_bytes));
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
QpJacobianOperator::operator()(BlockScratchLoop, const ThreadID tid, std::size_t & bytes) const
{
  auto elem = _thread(tid, 0);
  auto var = _thread(tid, 1);

  auto info = _level.kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;

  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  const auto needed = blockScratchBytes(kokkosAssembly().getNumQps(info));

  if (needed > bytes)
    bytes = needed;
}

KOKKOS_FUNCTION void
QpJacobianOperator::operator()(BlockLoop, const BlockTeam & team) const
{
  const ThreadID tid = team.league_rank();

  auto elem = _thread(tid, 0);
  auto var = _thread(tid, 1);

  auto info = _level.kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;

  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  const auto num_qps = kokkosAssembly().getNumQps(info);
  const auto qp_offset = kokkosAssembly().getQpOffset(info);

  const LevelBasisTable basis(
      kokkosAssembly(), sid, info.type, _level.getFEType(var), info.orientation);

  const auto num_dofs = basis.numDofs();

  // Every pair of degrees of freedom sharing a block sums over the same quadrature points, so the
  // element's linearization is staged here and read once rather than once per pair. It is private to
  // this element, which is what makes scratch the right place for it; the basis tables below are
  // identical for every element of a type and belong in the cache that already serves them.
  ScratchTensor tensors(team.team_scratch(0), num_qps);

  ::Kokkos::parallel_for(::Kokkos::TeamThreadRange(team, num_qps),
                         [&](const unsigned int qp)
                         { tensors(qp) = _cache.getTensor(sid, var, qp_offset + qp); });

  team.team_barrier();

  // One thread per ordered pair of the element's degrees of freedom. The decomposition holds no
  // fixed row and no row this process does not own, so the pair tests are what confine the
  // contraction to the entries of a block this process is building. An entity's block gathers a
  // contribution from every element carrying the entity, which for an interior block is one element
  // and for a shared edge or face is each element on it.
  ::Kokkos::parallel_for(
      ::Kokkos::TeamThreadRange(team, num_dofs * num_dofs),
      [&](const unsigned int pair)
      {
        const auto i = pair / num_dofs;

        const auto row = _level.getElemLocalDofIndex(elem, i, var);

        if (!_blocks.isBlocked(row))
          return;

        const auto j = pair % num_dofs;

        const auto col = _level.getElemLocalDofIndex(elem, j, var);

        if (!_blocks.isBlocked(col))
          return;

        const auto block = _blocks.blockOfDof(row);

        if (_blocks.blockOfDof(col) != block)
          return;

        Real entry = 0;

        for (unsigned int qp = 0; qp < num_qps; ++qp)
        {
          const auto & tensor = tensors(qp);

          Real value_i, value_j;
          Real3 grad_i, grad_j;

          basis.basis(i, qp, value_i, grad_i);
          basis.basis(j, qp, value_j, grad_j);

          entry += value_i * (tensor.vv * value_j + tensor.vg * grad_j) +
                   grad_i * (tensor.gv * value_j + tensor.gg * grad_j);
        }

        ::Kokkos::atomic_add(&_blocks.entry(block, _blocks.posOfDof(row), _blocks.posOfDof(col)),
                             entry);
      });
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
