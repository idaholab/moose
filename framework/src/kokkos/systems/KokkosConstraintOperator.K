//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConstraintOperator.h"

#include "libmesh/dof_map.h"
#include "libmesh/petsc_vector.h"

namespace Moose::Kokkos
{

void
ConstraintOperator::setup(const libMesh::DofMap & dof_map,
                          libMesh::NumericVector<Number> & solution_vector,
                          const dof_id_type num_local_plus_ghost,
                          const std::unordered_map<dof_id_type, bool> & preset_dofs)
{
  auto * const solution = libMesh::cast_ptr<libMesh::PetscVector<Number> *>(&solution_vector);

  const auto & constraints = dof_map.get_dof_constraints();
  auto & values = const_cast<libMesh::DofMap &>(dof_map).get_primal_constraint_values();

  std::vector<dof_id_type> rows;
  std::vector<dof_id_type> row_offsets = {0};
  std::vector<dof_id_type> cols;
  std::vector<Real> coeffs;
  std::vector<Real> inhomogeneity;
  std::vector<bool> preset;

  for (const auto & [row_dof, row] : constraints)
  {
    rows.push_back(solution->map_global_to_local_index(row_dof));

    for (const auto & [col_dof, coeff] : row)
    {
      cols.push_back(solution->map_global_to_local_index(col_dof));
      coeffs.push_back(coeff);
    }

    row_offsets.push_back(cols.size());

    const auto value_it = values.find(row_dof);
    inhomogeneity.push_back(value_it == values.end() ? 0.0 : value_it->second);

    const auto preset_it = preset_dofs.find(row_dof);
    preset.push_back(preset_it == preset_dofs.end() ? false : preset_it->second);
  }

  _rows = rows;
  _row_offsets = row_offsets;
  _cols = cols;
  _coeffs = coeffs;
  _inhomogeneity = inhomogeneity;

  _row_preset.create(preset.size());
  for (const auto i : index_range(preset))
    _row_preset[i] = preset[i];
  _row_preset.copyToDevice();

  _constrained_mask.create(num_local_plus_ghost);
  _constrained_mask = false;
  for (const auto local_dof : rows)
    _constrained_mask[local_dof] = true;
  _constrained_mask.copyToDevice();
}

void
ConstraintOperator::presetSolution(Vector & solution)
{
  _preset_only = true;

  constrainSolution(solution);
}

void
ConstraintOperator::enforceConstraints(Vector & solution)
{
  _preset_only = false;

  constrainSolution(solution);
}

void
ConstraintOperator::constrainSolution(Vector & solution)
{
  if (!_rows.size())
    return;

  _vector = solution;

  ::Kokkos::parallel_for(
      ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, ConstrainLoop>(0,
                                                                                     _rows.size()),
      *this);
  ::Kokkos::fence();
}

void
ConstraintOperator::finalizeResidual(Vector & residual, const Vector & solution)
{
  if (!_rows.size())
    return;

  _vector = residual;
  _vector2 = solution;

  ::Kokkos::parallel_for(
      ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, ResidualLoop>(0,
                                                                                    _rows.size()),
      *this);
  ::Kokkos::fence();
}

void
ConstraintOperator::finalizeJacobianVectorProduct(Vector & y, const Vector & x)
{
  if (!_rows.size())
    return;

  _vector = y;
  _vector2 = x;

  ::Kokkos::parallel_for(
      ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, JVPLoop>(0, _rows.size()),
      *this);
  ::Kokkos::fence();
}

void
ConstraintOperator::finalizeDiagonal(Vector & diagonal, const Real value)
{
  if (!_rows.size())
    return;

  _vector = diagonal;
  _value = value;

  ::Kokkos::parallel_for(
      ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, DiagonalLoop>(0,
                                                                                    _rows.size()),
      *this);
  ::Kokkos::fence();
}

KOKKOS_FUNCTION void
ConstraintOperator::operator()(ConstrainLoop, const ThreadID tid) const
{
  if (_preset_only && !_row_preset[tid])
    return;

  Real v = _inhomogeneity[tid];

  for (auto k = _row_offsets[tid]; k < _row_offsets[tid + 1]; ++k)
    v += _coeffs[k] * _vector[_cols[k]];

  _vector[_rows[tid]] = v;
}

KOKKOS_FUNCTION void
ConstraintOperator::operator()(ResidualLoop, const ThreadID tid) const
{
  // The row's own equation, u_i - g_i - sum_j c_ij u_j, evaluated at the current solution. A preset
  // row's solution already satisfies this exactly (presetSolution() put it there), so this comes
  // out zero without needing to special-case it; a non-preset row's value is the real, nonzero
  // residual the solve still has to drive to zero.
  Real v = _vector2[_rows[tid]] - _inhomogeneity[tid];

  for (auto k = _row_offsets[tid]; k < _row_offsets[tid + 1]; ++k)
    v -= _coeffs[k] * _vector2[_cols[k]];

  _vector[_rows[tid]] = v;
}

KOKKOS_FUNCTION void
ConstraintOperator::operator()(JVPLoop, const ThreadID tid) const
{
  // The row's own constraint equation's directional derivative, x_i - sum_j c_ij x_j, with no
  // distinction between a preset and a non-preset row. This loop only replaces this row's own
  // action; a free row's action never carried this DOF's direction in the first place, since the
  // operator leaves a constrained DOF out of its trial-space gather.
  Real v = _vector2[_rows[tid]];

  for (auto k = _row_offsets[tid]; k < _row_offsets[tid + 1]; ++k)
    v -= _coeffs[k] * _vector2[_cols[k]];

  _vector[_rows[tid]] = v;
}

KOKKOS_FUNCTION void
ConstraintOperator::operator()(DiagonalLoop, const ThreadID tid) const
{
  _vector[_rows[tid]] = _value;
}

} // namespace Moose::Kokkos
