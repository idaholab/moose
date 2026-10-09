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
 * value and of its physical gradient, followed by their derivatives with respect to each slot:
 *
 *   [qp][0]                 f_value
 *   [qp][1 .. 3]            f_grad
 *   [qp][4 (1 + s)]         d f_value / d slot s
 *   [qp][4 (1 + s) + 1 .. 3] d f_grad / d slot s
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
    return term_size * (1 + slots);
  }

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
 * Evaluate the residual hooks of a pointwise AD residual object at one quadrature point and store
 * the JxW-weighted coefficients and their slot derivatives into scratch
 * @param object The residual object of the final derived type
 * @param datum The AssemblyDatum object of the element or side
 * @param qp The local quadrature point index
 * @param scratch The scratch entries of the quadrature point
 */
template <typename Object>
KOKKOS_FUNCTION void
evaluateADTerms(const Object & object, AssemblyDatum & datum, const unsigned int qp, Real * scratch)
{
  constexpr auto slots = Object::max_ad_slots;
  const auto JxW = datum.JxW(qp);

  for (unsigned int k = 0; k < ADScratchLayout::qpSize(slots); ++k)
    scratch[k] = 0;

  if constexpr (has_precomputeQpResidualValue_v<Object>)
    storeADCoefficient<slots>(
        scratch, 0, JxW * object.template precomputeQpResidualValue<Object>(qp, datum));

  if constexpr (has_precomputeQpResidualGrad_v<Object>)
  {
    const auto grad = object.template precomputeQpResidualGrad<Object>(qp, datum);
    for (unsigned int k = 0; k < 3; ++k)
      storeADCoefficient<slots>(scratch, 1 + k, JxW * grad(k));
  }
}

/**
 * Contract the coefficients of one term set at a quadrature point with a shape function
 * @param terms The term set: value coefficient followed by the gradient coefficient
 * @param phi The shape function value
 * @param grad_phi The physical shape function gradient
 */
KOKKOS_INLINE_FUNCTION Real
contractADTerms(const Real * terms, const Real phi, const Real3 & grad_phi)
{
  return terms[0] * phi + terms[1] * grad_phi(0) + terms[2] * grad_phi(1) + terms[3] * grad_phi(2);
}

} // namespace Moose::Kokkos
