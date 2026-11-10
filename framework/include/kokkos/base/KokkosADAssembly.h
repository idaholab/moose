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
 * Layout of the scratch memory of one group of a pointwise AD element or side assembly. At each
 * quadrature point the scratch holds the JxW-weighted residual coefficients of the test function
 * value and of its reference gradient, followed by their derivatives with respect to each slot:
 *
 *   [qp][0]                  f_value
 *   [qp][1 .. 3]             f_grad, mapped to the reference element
 *   [qp][4 (1 + s)]          d f_value / d slot s
 *   [qp][4 (1 + s) + 1 .. 3] d f_grad / d slot s
 *
 * The gradient coefficient is contracted with the reference test function gradient, so that the
 * variation of the test function gradient with the displacements is carried by the derivatives of
 * the mapped coefficient. The derivatives with respect to a gradient slot block are mapped to the
 * reference element as well, so that they contract with the reference trial function gradient.
 *
 * The Jacobian contraction follows, in four planes, one per entry of a term set. For a chunk of
 * quadrature points and one column variable, plane t holds at [qp][j] the derivatives of entry t
 * contracted with the slot quantities of trial function j.
 */
struct ADScratchLayout
{
  /// Number of reals per term set: one value coefficient and three gradient components
  static constexpr unsigned int term_size = 4;

  /**
   * Get the number of reals stored per quadrature point. The count is odd, so that the stores of
   * the quadrature points of a warp fall in distinct shared memory banks.
   * @param slots The number of derivative slots
   */
  KOKKOS_FUNCTION static constexpr unsigned int qpSize(unsigned int slots)
  {
    return term_size * (1 + slots) + 1;
  }

  /**
   * Get the offset of the Jacobian contraction planes within the scratch of a group
   */
  KOKKOS_FUNCTION unsigned int jacobianOffset() const { return max_qps * qpSize(slots); }
  /**
   * Get the number of reals of one Jacobian contraction plane
   */
  KOKKOS_FUNCTION unsigned int jacobianPlane() const { return qp_chunk * max_jdofs; }
  /**
   * Get the number of reals of the scratch of a group
   */
  KOKKOS_FUNCTION unsigned int groupSize() const
  {
    return jacobianOffset() + term_size * jacobianPlane();
  }
  /**
   * Get the number of bytes of scratch memory per team
   * @param groups The number of groups of the team, each assembling one element or side
   */
  std::size_t bytes(unsigned int groups) const
  {
    return ::Kokkos::View<Real *, ::Kokkos::DefaultExecutionSpace::scratch_memory_space>::
        shmem_size(groups * groupSize());
  }

  /// Maximum number of quadrature points of an element or side
  unsigned int max_qps = 0;
  /// Number of derivative slots
  unsigned int slots = 0;
  /// Maximum number of DOFs of a column variable, or zero without a Jacobian contraction
  unsigned int max_jdofs = 0;
  /// Number of quadrature points per chunk of the Jacobian contraction
  unsigned int qp_chunk = 0;
};

/**
 * Partition of a team of a pointwise AD assembly into groups of threads. Each group assembles one
 * element or side, so that a team of short quadrature point loops keeps its threads busy. A group
 * holds one thread per quadrature point, up to the team size.
 */
class ADTeamGroup
{
public:
  /// Number of threads per team that a dispatch aims for, which the backend limits may lower
  static constexpr unsigned int target_team_size = 128;

  /**
   * Get the number of threads per group
   * @param max_qps The maximum number of quadrature points of an element or side
   * @param team_size The number of threads per team
   */
  KOKKOS_FUNCTION static unsigned int groupSize(const unsigned int max_qps,
                                                const unsigned int team_size)
  {
    return ::Kokkos::max(1u, ::Kokkos::min(max_qps, team_size));
  }

  /**
   * Constructor
   * @param team The team handle
   * @param max_qps The maximum number of quadrature points of an element or side
   * @param n_items The number of elements or sides of the dispatch
   */
  template <typename Team>
  KOKKOS_FUNCTION ADTeamGroup(const Team & team, const unsigned int max_qps, const ThreadID n_items)
    : _max_qps(max_qps),
      _size(groupSize(max_qps, team.team_size())),
      _count(team.team_size() / _size),
      _index(team.team_rank() / _size),
      _rank(team.team_rank() % _size),
      _item(ThreadID(team.league_rank()) * _count + _index),
      _active(_item < n_items)
  {
    // A thread without an element or side still reaches the team barriers, and reads a valid one
    if (!_active)
      _item = n_items - 1;
  }

  /**
   * Get the index of the element or side of the group within the dispatch
   */
  KOKKOS_FUNCTION ThreadID item() const { return _item; }
  /**
   * Get whether the group assembles its element or side
   */
  KOKKOS_FUNCTION bool active() const { return _active; }
  /**
   * Get the number of groups of the team
   */
  KOKKOS_FUNCTION unsigned int count() const { return _count; }
  /**
   * Get the index of the group within the team
   */
  KOKKOS_FUNCTION unsigned int index() const { return _index; }
  /**
   * Get the rank of the thread within the group
   */
  KOKKOS_FUNCTION unsigned int rank() const { return _rank; }
  /**
   * Get the number of threads of the group
   */
  KOKKOS_FUNCTION unsigned int size() const { return _size; }
  /**
   * Get the maximum number of quadrature points of an element or side, which strides the scratch
   * of the groups
   */
  KOKKOS_FUNCTION unsigned int maxQps() const { return _max_qps; }

private:
  /// Maximum number of quadrature points of an element or side
  const unsigned int _max_qps;
  /// Number of threads per group
  const unsigned int _size;
  /// Number of groups per team
  const unsigned int _count;
  /// Index of the group within the team
  const unsigned int _index;
  /// Rank of the thread within the group
  const unsigned int _rank;
  /// Index of the element or side of the group within the dispatch
  ThreadID _item;
  /// Whether the group assembles its element or side
  const bool _active;
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

  // An absent hook contributes zero coefficients
  if constexpr (HasADResidualValue<Object, slots>::value)
    storeADCoefficient<slots>(
        scratch, 0, JxW * object.template precomputeQpResidualValue<Object, slots>(qp, datum));
  else
    storeADCoefficient<slots>(scratch, 0, DenseADReal<slots>(0));

  if constexpr (HasADResidualGrad<Object, slots>::value)
  {
    const DenseADReal3<slots> grad = datum.adToReferenceElement(
        JxW * object.template precomputeQpResidualGrad<Object, slots>(qp, datum), qp);
    for (unsigned int k = 0; k < 3; ++k)
      storeADCoefficient<slots>(scratch, 1 + k, grad(k));
  }
  else
    for (unsigned int k = 0; k < 3; ++k)
      storeADCoefficient<slots>(scratch, 1 + k, DenseADReal<slots>(0));

  // A gradient slot holds a component of the gradient on the reference configuration, whose
  // variation with a DOF is the reference trial function gradient mapped by dxi/dx. Applying the
  // map to the derivatives once per quadrature point lets them contract with the reference trial
  // function gradient directly.
  if constexpr (slots > 0)
  {
    constexpr auto term_size = ADScratchLayout::term_size;
    const auto & dxi_dx = datum.reference_dxi_dx(qp);
    const auto & table = datum.table();

    for (unsigned int b = 0; b < table.numBlocks(); ++b)
      if (const auto & block = table.block(b); block.gradient)
        for (unsigned int t = 0; t < term_size; ++t)
        {
          Real * d = scratch + term_size * (1 + block.first) + t;
          const Real d0 = d[0];
          const Real d1 = d[term_size];
          const Real d2 = d[2 * term_size];

          for (unsigned int m = 0; m < 3; ++m)
            d[m * term_size] = d0 * dxi_dx(0, m) + d1 * dxi_dx(1, m) + d2 * dxi_dx(2, m);
        }
  }
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
