//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosQpJacobianOperator.h"

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

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, ApplyLoop> policy(0,
                                                                                    _thread.size());
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();
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

KOKKOS_FUNCTION void
QpJacobianOperator::operator()(ApplyLoop, const ThreadID tid) const
{
  auto elem = _thread(tid, 0);
  auto var = _thread(tid, 1);

  auto info = _level.kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;

  // A pair with no declared blocks has no cached linearization; the objects responsible for it
  // supply their own operator action
  if (_cache.getBlocks(sid, var) == QP_JACOBIAN_NONE)
    return;

  auto num_qps = kokkosAssembly().getNumQps(info);
  auto qp_offset = kokkosAssembly().getQpOffset(info);

  const LevelBasisTable basis(
      kokkosAssembly(), sid, info.type, _level.getFEType(var), info.orientation);

  const auto num_dofs = basis.numDofs();

  // Where the level eliminates a fixed column, that row's value is not an unknown of the
  // linearization and its direction is zero, so leaving it out of the trial-space gather is what
  // makes the action this loop applies symmetric whenever the linearization it contracts is, and
  // makes this loop and the MatrixLoop below the same operator. Where the level does not, the
  // residual being linearized still reads that row, and dropping it here would make the action
  // disagree with the residual instead.
  const auto gather = [&](const unsigned int j) -> Real
  {
    const auto dof = _level.getElemLocalDofIndex(elem, j, var);

    return _level.isEliminatedColumn(dof) ? 0.0 : _level.getDofValue(dof, _x_tag);
  };

  Real local_y[MAX_CACHED_DOF];

  // The test function index is batched so that the per-element accumulator stays within a bounded
  // amount of per-thread storage. The trial-space gather is repeated for each batch, which costs
  // nothing for the element types whose DOF count fits in a single batch.
  for (unsigned int ib = 0; ib < num_dofs; ib += MAX_CACHED_DOF)
  {
    const auto ie = ::Kokkos::min(ib + MAX_CACHED_DOF, num_dofs);

    for (unsigned int i = ib; i < ie; ++i)
      local_y[i - ib] = 0;

    for (unsigned int qp = 0; qp < num_qps; ++qp)
    {
      // The direction vector in the reference trial space at this quadrature point
      Real value;
      Real3 grad;

      basis.applyB(gather, qp, value, grad);

      const auto & tensor = _cache.getTensor(sid, var, qp_offset + qp);

      // The linearized value/flux pair the test functions are contracted against
      const Real f = tensor.vv * value + tensor.vg * grad;
      const Real3 flux = tensor.gv * value + tensor.gg * grad;

      basis.applyBT(qp, f, flux, ib, ie, local_y);
    }

    for (unsigned int i = ib; i < ie; ++i)
    {
      auto row = _level.getElemLocalDofIndex(elem, i, var);

      if (_level.isConstrained(row))
        continue;

      ::Kokkos::atomic_add(&_level.getDofValue(row, _y_tag), local_y[i - ib]);
    }
  }
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

} // namespace Moose::Kokkos
