//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_LIBTORCH_ENABLED

#include <cmath>
#include <ATen/ATen.h>

/**
 * Small raw-ATen helpers for the batched finite-element assembly used by the Torch FEM kernels.
 *
 * These reproduce the trivial torch operations that NEML2 v2 exposed through its (now-deleted) C++
 * typed-tensor API (neml2::discretization::{scatter,interpolate,assemble} and the SR2<->R2 Mandel
 * conversions). All tensors are plain at::Tensor with explicit batch dimensions; the leading dims
 * are batch (nelem, ndofe, nqp, ...) and any trailing dims are the tensor base shape.
 */
namespace TorchFEM
{
/// Gather global solution values into element-local DOF values: out[e,a,...] = v[dof_map[e,a,...]].
/// @param v Flat (local) solution vector, shape (local_ndof,).
/// @param dof_map Integer local-dof indices, shape (nelem, ndofe).
/// @returns Element-local values, shape == dof_map.shape (nelem, ndofe).
inline at::Tensor
scatter(const at::Tensor & v, const at::Tensor & dof_map)
{
  return v.index({dof_map});
}

/// Interpolate element-local DOF values onto quadrature points by contracting over the per-element
/// DOF axis (dim 1) against a basis: out[e,...] = sum_a elem_dofs[e,a] * basis[e,a,...].
/// @param elem_dofs Element-local values, shape (nelem, ndofe).
/// @param basis Shape functions (nelem, ndofe, nqp) or their gradients (nelem, ndofe, nqp, 3).
/// @returns (nelem, nqp) for values, (nelem, nqp, 3) for gradients.
inline at::Tensor
interpolate(const at::Tensor & elem_dofs, const at::Tensor & basis)
{
  auto ed = elem_dofs;
  while (ed.dim() < basis.dim())
    ed = ed.unsqueeze(-1);
  return (ed * basis).sum(1);
}

/// Scatter-add element-local contributions into a global vector:
/// out[dof_map.flatten()[k]] += scattered.flatten()[k].
/// @param scattered Element-local contributions, same shape as dof_map.
/// @param dof_map Integer local-dof indices, same shape as scattered.
/// @param ndof Length of the assembled global vector.
inline at::Tensor
assemble(const at::Tensor & scattered, const at::Tensor & dof_map, int64_t ndof)
{
  return at::zeros({ndof}, scattered.options())
      .scatter_add_(0, dof_map.flatten(), scattered.flatten());
}

/// Convert a full (symmetric) rank-2 tensor to its 6-component Mandel representation, matching
/// NEML2's SR2 convention: components [00, 11, 22, sqrt(2)*12, sqrt(2)*02, sqrt(2)*01]. The caller
/// is responsible for symmetrizing the input if needed (mirrors neml2::SR2(R2), which uses the
/// symmetric part).
/// @param full (..., 3, 3).
/// @returns (..., 6).
inline at::Tensor
fullToMandel(const at::Tensor & full)
{
  const double s2 = std::sqrt(2.0);
  auto c = [&full](int i, int j) { return full.select(-2, i).select(-1, j); };
  return at::stack({c(0, 0), c(1, 1), c(2, 2), s2 * c(1, 2), s2 * c(0, 2), s2 * c(0, 1)}, -1);
}

/// Inverse of fullToMandel: expand a 6-component Mandel tensor to a full symmetric (..., 3, 3),
/// matching neml2::R2(SR2) (off-diagonals divided by sqrt(2)).
/// @param mandel (..., 6).
/// @returns (..., 3, 3).
inline at::Tensor
mandelToFull(const at::Tensor & mandel)
{
  const double is2 = 1.0 / std::sqrt(2.0);
  auto m = [&mandel](int k) { return mandel.select(-1, k); };
  auto d0 = m(0), d1 = m(1), d2 = m(2);
  auto o12 = m(3) * is2, o02 = m(4) * is2, o01 = m(5) * is2;
  auto row0 = at::stack({d0, o01, o02}, -1);
  auto row1 = at::stack({o01, d1, o12}, -1);
  auto row2 = at::stack({o02, o12, d2}, -1);
  return at::stack({row0, row1, row2}, -2);
}
} // namespace TorchFEM

#endif // MOOSE_LIBTORCH_ENABLED
