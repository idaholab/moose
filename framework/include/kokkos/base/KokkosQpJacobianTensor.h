//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"

namespace Moose::Kokkos
{

/**
 * The linearization of a weak form at a single quadrature point, expressed against
 * reference-element basis functions.
 *
 * A residual contribution in value/flux form is
 *
 * $r_i = \sum_q JxW_q \left( \psi_i f_q + \nabla\psi_i \cdot F_q \right)$
 *
 * where the value $f_q$ and the flux $F_q$ depend on the solution only through its value $u_q$ and
 * gradient $\nabla u_q$ at that same quadrature point. Differentiating with respect to that pair
 * gives four blocks, named here for the (test slot, trial slot) they connect: a scalar value-value
 * block, two value-gradient vector blocks, and a gradient-gradient tensor block.
 *
 * The quadrature weight, the Jacobian determinant, and the geometric mapping are all folded into
 * the stored blocks, so the gradient slots refer to *reference*-element gradients: a consumer
 * contracts the blocks directly with reference basis tables and never touches element geometry.
 * Because those tables are the only level-dependent input, one fill of this tensor on the fine
 * level supplies the operator action and diagonal for every level of a p-multigrid hierarchy whose
 * levels share the quadrature rule.
 */
struct QpJacobianTensor
{
  /// Test value against trial value
  Real vv;
  /// Test value against trial reference gradient
  Real3 vg;
  /// Test reference gradient against trial value
  Real3 gv;
  /// Test reference gradient against trial reference gradient
  Real33 gg;

#ifdef MOOSE_KOKKOS_SCOPE
  /// Zero every block
  KOKKOS_INLINE_FUNCTION void zero()
  {
    vv = 0;
    vg = 0;
    gv = 0;
    gg = 0;
  }

  /// Accumulate another tensor's blocks into this one
  KOKKOS_INLINE_FUNCTION void operator+=(const QpJacobianTensor & tensor)
  {
    vv += tensor.vv;
    vg += tensor.vg;
    gv += tensor.gv;
    gg += tensor.gg;
  }
#endif
};

/**
 * Identifies which blocks of a QpJacobianTensor a kernel's linearization can populate.
 *
 * A kernel declares its mask once, at construction; the union of the masks of the kernels active on
 * a subdomain says which blocks a consumer of the cache must contract there. Skipping the blocks no
 * kernel can reach keeps the contraction cost proportional to the physics rather than to the four
 * blocks a general weak form could occupy.
 */
enum QpJacobianBlock : unsigned int
{
  QP_JACOBIAN_NONE = 0,
  QP_JACOBIAN_VALUE_VALUE = 1 << 0,
  QP_JACOBIAN_VALUE_GRADIENT = 1 << 1,
  QP_JACOBIAN_GRADIENT_VALUE = 1 << 2,
  QP_JACOBIAN_GRADIENT_GRADIENT = 1 << 3
};

/**
 * The object a kernel writes its quadrature-point linearization into.
 *
 * The setters take *physical*-space quantities, which is the space a kernel's residual is written
 * in: the derivative of the value/flux pair with respect to the solution value and its physical
 * gradient. Folding those into the reference-space, weight-scaled blocks of a QpJacobianTensor is
 * done by fold(), so a kernel author writes the same derivatives they would write for an ordinary
 * element Jacobian and needs no knowledge of the element mapping or of the levels that will consume
 * the result.
 */
#ifdef MOOSE_KOKKOS_SCOPE

class QpJacobianBlockAccessor
{
public:
  KOKKOS_INLINE_FUNCTION QpJacobianBlockAccessor() { _tensor.zero(); }

  ///@{
  /**
   * Accumulate one block of $\partial (f_q, F_q) / \partial (u_q, \nabla u_q)$, in physical space
   * @param block The derivative of the named residual slot with respect to the named solution slot
   */
  /// $\partial f_q / \partial u_q$
  KOKKOS_INLINE_FUNCTION void addValueValue(const Real block) { _tensor.vv += block; }
  /// $\partial f_q / \partial \nabla u_q$
  KOKKOS_INLINE_FUNCTION void addValueGradient(const Real3 & block) { _tensor.vg += block; }
  /// $\partial F_q / \partial u_q$
  KOKKOS_INLINE_FUNCTION void addFluxValue(const Real3 & block) { _tensor.gv += block; }
  /// $\partial F_q / \partial \nabla u_q$
  KOKKOS_INLINE_FUNCTION void addFluxGradient(const Real33 & block) { _tensor.gg += block; }
  ///@}

  ///@{
  /**
   * Read one accumulated block, in physical space
   * @returns The derivative of the named residual slot with respect to the named solution slot
   */
  KOKKOS_INLINE_FUNCTION Real valueValue() const { return _tensor.vv; }
  KOKKOS_INLINE_FUNCTION const Real3 & valueGradient() const { return _tensor.vg; }
  KOKKOS_INLINE_FUNCTION const Real3 & fluxValue() const { return _tensor.gv; }
  KOKKOS_INLINE_FUNCTION const Real33 & fluxGradient() const { return _tensor.gg; }
  ///@}

  /**
   * Convert the accumulated physical-space blocks into a reference-space, weight-scaled tensor
   * @param jacobian The mapping from reference to physical gradients at this quadrature point,
   * i.e. AssemblyDatum::J()
   * @param jxw The quadrature weight times the Jacobian determinant at this quadrature point
   * @returns The tensor to accumulate into the cache
   */
  KOKKOS_INLINE_FUNCTION QpJacobianTensor fold(const Real33 & jacobian, const Real jxw) const
  {
    // A physical trial gradient is J times the reference trial gradient, and a physical test
    // gradient contracted against a flux is the reference test gradient contracted against J
    // transpose times that flux, so each gradient slot picks up one factor of J on the
    // corresponding side.
    const Real33 jacobian_t = jacobian.transpose();

    QpJacobianTensor tensor;
    tensor.vv = jxw * _tensor.vv;
    tensor.vg = jxw * (jacobian_t * _tensor.vg);
    tensor.gv = jxw * (jacobian_t * _tensor.gv);
    tensor.gg = jxw * (jacobian_t * _tensor.gg * jacobian);

    return tensor;
  }

private:
  /// The physical-space blocks accumulated by the setters
  QpJacobianTensor _tensor;
};

#endif

} // namespace Moose::Kokkos
