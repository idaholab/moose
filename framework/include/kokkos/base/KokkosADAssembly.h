//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADResidualObject.h"
#include "KokkosPrecomputeHookTraits.h"
#include "KokkosDispatcher.h"

namespace Moose::Kokkos
{

/**
 * Layout of the per-team scratch memory of a pointwise AD element or side assembly. At each
 * quadrature point the scratch holds the JxW-weighted residual coefficients of the test function
 * value and of its reference gradient, followed by their derivatives with respect to each slot,
 * and then the inverse of the reference map Jacobian:
 *
 *   [qp][0]                 f_value
 *   [qp][1 .. 3]            f_grad, pulled back to the reference element
 *   [qp][4 (1 + s)]         d f_value / d slot s
 *   [qp][4 (1 + s) + 1 .. 3] d f_grad / d slot s
 *   [qp][4 (1 + slots) ..]  dxi/dx of the reference configuration, row major
 *
 * The gradient coefficient is contracted with the reference test function gradient, so that the
 * variation of the test function gradient with the displacements is carried by the derivatives of
 * the pulled back coefficient. The trial functions of the slot blocks are taken on the reference
 * configuration, where the gradient slots are defined.
 */
struct ADScratchLayout
{
  /// Number of reals per term set: one value coefficient and three gradient components
  static constexpr unsigned int term_size = 4;

  /**
   * Get the number of reals stored per quadrature point
   * @param slots The number of derivative slots
   */
  KOKKOS_FUNCTION static constexpr unsigned int qpSize(unsigned int slots)
  {
    return term_size * (1 + slots) + map_size;
  }
  /**
   * Get the offset of the inverse of the reference map Jacobian within the entries of a
   * quadrature point
   * @param slots The number of derivative slots
   */
  KOKKOS_FUNCTION static constexpr unsigned int mapOffset(unsigned int slots)
  {
    return term_size * (1 + slots);
  }

  /// Number of reals of the inverse of the reference map Jacobian
  static constexpr unsigned int map_size = 9;

  /**
   * Get the number of bytes of scratch memory per team
   * @param max_qps The maximum number of quadrature points of an element or side
   * @param slots The number of derivative slots
   */
  static std::size_t bytes(unsigned int max_qps, unsigned int slots)
  {
    return ::Kokkos::View<Real *, ::Kokkos::DefaultExecutionSpace::scratch_memory_space>::
        shmem_size(max_qps * qpSize(slots));
  }
};

/**
 * View of the per-team scratch memory of a pointwise AD element or side assembly
 */
using ADScratchView = ::Kokkos::
    View<Real *, ExecSpace::scratch_memory_space, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>>;

/**
 * Store a residual coefficient and its slot derivatives into scratch
 * @param scratch The scratch entries of the quadrature point
 * @param offset The offset of the coefficient within a term set
 * @param coef The JxW-weighted coefficient
 */
template <unsigned int slots>
KOKKOS_FUNCTION void
storeADCoefficient(Real * scratch, const unsigned int offset, const DenseADReal<slots> & coef)
{
  scratch[offset] = denseADValue(coef);

  for (unsigned int s = 0; s < slots; ++s)
    scratch[ADScratchLayout::term_size * (1 + s) + offset] = denseADDerivative(coef, s);
}

/**
 * Detection of the residual hooks of a pointwise AD residual object at a number of derivative
 * slots
 */
///@{
template <typename Object, unsigned int slots, typename = void>
struct HasADResidualValue : std::false_type
{
};
template <typename Object, unsigned int slots>
struct HasADResidualValue<
    Object,
    slots,
    std::void_t<
        decltype(std::declval<const Object &>().template precomputeQpResidualValue<Object, slots>(
            0u, std::declval<ADAssemblyDatum<slots> &>()))>> : std::true_type
{
};
template <typename Object, unsigned int slots, typename = void>
struct HasADResidualGrad : std::false_type
{
};
template <typename Object, unsigned int slots>
struct HasADResidualGrad<
    Object,
    slots,
    std::void_t<
        decltype(std::declval<const Object &>().template precomputeQpResidualGrad<Object, slots>(
            0u, std::declval<ADAssemblyDatum<slots> &>()))>> : std::true_type
{
};
///@}

/**
 * Evaluate the residual hooks of a pointwise AD residual object at one quadrature point and store
 * the JxW-weighted coefficients and their slot derivatives into scratch
 * @param object The residual object of the final derived type
 * @param datum The ADAssemblyDatum object of the quadrature point
 * @param qp The local quadrature point index
 * @param scratch The scratch entries of the quadrature point
 */
template <typename Object, unsigned int slots>
KOKKOS_FUNCTION void
evaluateADTerms(const Object & object,
                ADAssemblyDatum<slots> & datum,
                const unsigned int qp,
                Real * scratch)
{
  const auto JxW = datum.adJxW(qp);

  for (unsigned int k = 0; k < ADScratchLayout::qpSize(slots); ++k)
    scratch[k] = 0;

  if constexpr (HasADResidualValue<Object, slots>::value)
    storeADCoefficient<slots>(
        scratch, 0, JxW * object.template precomputeQpResidualValue<Object, slots>(qp, datum));

  if constexpr (HasADResidualGrad<Object, slots>::value)
  {
    const DenseADReal3<slots> grad = datum.adPullBack(
        JxW * object.template precomputeQpResidualGrad<Object, slots>(qp, datum), qp);
    for (unsigned int k = 0; k < 3; ++k)
      storeADCoefficient<slots>(scratch, 1 + k, grad(k));
  }

  const auto & dxi_dx = datum.reference_dxi_dx(qp);
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      scratch[ADScratchLayout::mapOffset(slots) + 3 * i + j] = dxi_dx(i, j);
}

/**
 * Contract the coefficients of one term set at a quadrature point with a shape function
 * @param terms The term set: value coefficient followed by the gradient coefficient
 * @param phi The shape function value
 * @param grad_phi The shape function gradient matching the gradient coefficient
 */
KOKKOS_INLINE_FUNCTION Real
contractADTerms(const Real * terms, const Real phi, const Real3 & grad_phi)
{
  return terms[0] * phi + terms[1] * grad_phi(0) + terms[2] * grad_phi(1) + terms[3] * grad_phi(2);
}

} // namespace Moose::Kokkos
