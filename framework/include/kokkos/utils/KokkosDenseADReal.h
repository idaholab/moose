//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include "metaphysicl/dualnumberarray.h"

#include <type_traits>

namespace Moose::Kokkos
{

/**
 * Dual number for pointwise automatic differentiation at a quadrature point. Its derivatives are
 * taken with respect to the quadrature point inputs a residual object reads, its slots: one slot
 * for the value of each coupled scalar field, three for its gradient, and so on. A residual object
 * declares the number of slots it reads as max_ad_slots, which sizes this type at compile time. A
 * residual object that reads no inputs computes with plain reals.
 */
///@{
template <unsigned int slots>
struct DenseADRealType
{
  using type = MetaPhysicL::DualNumber<Real, MetaPhysicL::NumberArray<slots, Real>, false>;
};
template <>
struct DenseADRealType<0>
{
  using type = Real;
};
template <unsigned int slots>
using DenseADReal = typename DenseADRealType<slots>::type;
///@}

/**
 * Whether a type is a dual number
 */
///@{
template <typename T>
struct IsDualNumber : std::false_type
{
};
template <typename T, typename D, bool asd>
struct IsDualNumber<MetaPhysicL::DualNumber<T, D, asd>> : std::true_type
{
};
template <typename T>
inline constexpr bool is_dual_number = IsDualNumber<std::decay_t<T>>::value;
///@}

/**
 * Number of slots of each quadrature point quantity of a field
 */
///@{
constexpr unsigned int AD_VALUE_SLOTS = 1;
constexpr unsigned int AD_GRADIENT_SLOTS = 3;
///@}

/**
 * Set the value of a dense dual number and seed its derivatives with a single direction
 * @param value The value
 * @param slot The slot seeded, or libMesh::invalid_uint to seed nothing
 * @param seed The derivative of the value with respect to the slot
 * @returns The dual number
 */
template <unsigned int slots>
KOKKOS_INLINE_FUNCTION DenseADReal<slots>
seededDenseADReal(const Real value, const unsigned int slot, const Real seed)
{
  if constexpr (slots == 0)
    return value;
  else
  {
    DenseADReal<slots> dual = value;
    dual.derivatives() = 0;

    if (slot != libMesh::invalid_uint)
      dual.derivatives()[slot] = seed;

    return dual;
  }
}

/**
 * Get the derivative of a dense dual number with respect to a slot
 */
template <typename T>
KOKKOS_INLINE_FUNCTION Real
denseADDerivative(const T & dual, const unsigned int slot)
{
  if constexpr (std::is_same_v<T, Real>)
    return 0;
  else
    return dual.derivatives()[slot];
}

/**
 * Get the value of a dense dual number
 */
template <typename T>
KOKKOS_INLINE_FUNCTION Real
denseADValue(const T & dual)
{
  if constexpr (std::is_same_v<T, Real>)
    return dual;
  else
    return dual.value();
}

} // namespace Moose::Kokkos
