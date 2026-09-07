//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosQpJacobianTensor.h"

namespace Moose::Kokkos
{

#ifdef MOOSE_KOKKOS_SCOPE

/**
 * Quadrature-point automatic differentiation: seeding and harvesting.
 *
 * A weak form in value/flux form depends on the solution only through the pair
 * $(u_q, \nabla u_q)$ at the quadrature point being evaluated, so its linearization is obtained by
 * seeding $d + 1$ derivative directions on that pair and evaluating the physics once. The
 * derivative width is therefore set by the spatial dimension rather than by the number of element
 * degrees of freedom, which makes the cost of linearizing one quadrature point independent of the
 * polynomial order of the discretization.
 *
 * The seeds occupy derivative indices 0 (the value) and 1 through $d$ (the gradient components).
 * Those indices are meaningful only within a single seed-evaluate-harvest sequence: a quantity
 * carrying derivatives seeded some other way, such as a solution value read through an AD variable
 * accessor, indexes the same slots against element degrees of freedom, so the two must never meet
 * in one expression. The physics evaluated here is consequently written as a function of the seeded
 * pair it is handed.
 */
///@{
/// Derivative index the solution value is seeded on
constexpr unsigned int QP_AD_VALUE_INDEX = 0;
/// Derivative index the first solution gradient component is seeded on
constexpr unsigned int QP_AD_GRADIENT_INDEX = 1;

/**
 * Get one derivative entry of a quadrature-point-seeded quantity
 * @param value The evaluated quantity
 * @param index The derivative index, one of the seeded slots
 * @returns The derivative with respect to that seed, zero if the quantity does not depend on it
 */
KOKKOS_INLINE_FUNCTION Real
qpDerivative(const ADReal & value, const unsigned int index)
{
  const auto & derivatives = value.derivatives();

  for (std::size_t entry = 0; entry < derivatives.size(); ++entry)
    if (derivatives.raw_index(entry) == index)
      return derivatives.raw_at(entry);

  return 0;
}

/**
 * Seed the solution value at a quadrature point
 * @param value The solution value
 * @returns The seeded value
 */
KOKKOS_INLINE_FUNCTION ADReal
seedQpValue(const Real value)
{
  ADReal seeded = value;

  seeded.derivatives().insert(QP_AD_VALUE_INDEX) = 1;

  return seeded;
}

/**
 * Seed the solution gradient at a quadrature point
 * @param gradient The physical solution gradient
 * @param dim The spatial dimension
 * @returns The seeded gradient
 */
KOKKOS_INLINE_FUNCTION ADReal3
seedQpGradient(const Real3 & gradient, const unsigned int dim)
{
  ADReal3 seeded;

  seeded = gradient;

  for (unsigned int comp = 0; comp < dim; ++comp)
    seeded(comp).derivatives().insert(QP_AD_GRADIENT_INDEX + comp) = 1;

  return seeded;
}

/**
 * Harvest the linearization of the value part of a residual into the blocks it populates
 * @param value The value part, evaluated on seeded arguments
 * @param dim The spatial dimension
 * @param blocks The accessor to accumulate into
 */
KOKKOS_INLINE_FUNCTION void
harvestQpValue(const ADReal & value, const unsigned int dim, QpJacobianBlockAccessor & blocks)
{
  blocks.addValueValue(qpDerivative(value, QP_AD_VALUE_INDEX));

  Real3 value_gradient;

  for (unsigned int comp = 0; comp < dim; ++comp)
    value_gradient(comp) = qpDerivative(value, QP_AD_GRADIENT_INDEX + comp);

  blocks.addValueGradient(value_gradient);
}

/**
 * Harvest the linearization of the flux part of a residual into the blocks it populates
 * @param flux The flux part, evaluated on seeded arguments
 * @param dim The spatial dimension
 * @param blocks The accessor to accumulate into
 */
KOKKOS_INLINE_FUNCTION void
harvestQpFlux(const ADReal3 & flux, const unsigned int dim, QpJacobianBlockAccessor & blocks)
{
  Real3 flux_value;
  Real33 flux_gradient;

  for (unsigned int comp = 0; comp < dim; ++comp)
  {
    flux_value(comp) = qpDerivative(flux(comp), QP_AD_VALUE_INDEX);

    for (unsigned int grad_comp = 0; grad_comp < dim; ++grad_comp)
      flux_gradient(comp, grad_comp) = qpDerivative(flux(comp), QP_AD_GRADIENT_INDEX + grad_comp);
  }

  blocks.addFluxValue(flux_value);
  blocks.addFluxGradient(flux_gradient);
}
///@}

#endif

} // namespace Moose::Kokkos
