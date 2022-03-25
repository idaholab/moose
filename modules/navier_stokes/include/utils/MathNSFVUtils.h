//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MathFVUtils.h"

#include <vector>
#include <array>

namespace Moose
{
namespace FV
{
/**
 * std::vector overload
 */
template <typename Limiter, typename ValueType, typename GradientType>
std::vector<ValueType>
interpolate(const Limiter & limiter,
            const std::vector<ValueType> & phi_upwind,
            const std::vector<ValueType> & phi_downwind,
            const std::vector<GradientType> * const grad_phi_upwind,
            const FaceInfo & fi,
            const bool fi_elem_is_upwind)
{
  mooseAssert(limiter.constant() || grad_phi_upwind,
              "Non-null gradient only supported for constant limiters.");
  mooseAssert(!grad_phi_upwind || (grad_phi_upwind->size() == phi_upwind.size()),
              "Gradient and value container sizes must be the same.");
  mooseAssert(phi_upwind.size() == phi_downwind.size(),
              "Upwind and downwind value sizes must be the same.");

  std::vector<ValueType> ret;
  const GradientType * gradient = nullptr;
  for (const auto i : index_range(phi_upwind))
  {
    if (grad_phi_upwind)
      gradient = &(*grad_phi_upwind)[i];

    ret[i] = interpolate(limiter, phi_upwind[i], phi_downwind[i], gradient, fi, fi_elem_is_upwind);
  }

  return ret;
}

/**
 * std::array overload
 */
template <typename Limiter, typename ValueType, typename GradientType, std::size_t N>
std::array<ValueType, N>
interpolate(const Limiter & limiter,
            const std::array<ValueType, N> & phi_upwind,
            const std::array<ValueType, N> & phi_downwind,
            const std::array<GradientType, N> * const grad_phi_upwind,
            const FaceInfo & fi,
            const bool fi_elem_is_upwind)
{
  mooseAssert(limiter.constant() || grad_phi_upwind,
              "Non-null gradient only supported for constant limiters.");

  std::array<ValueType, N> ret;
  const GradientType * gradient = nullptr;
  for (const auto i : make_range(N))
  {
    if (grad_phi_upwind)
      gradient = &(*grad_phi_upwind)[i];

    ret[i] = interpolate(limiter, phi_upwind[i], phi_downwind[i], gradient, fi, fi_elem_is_upwind);
  }

  return ret;
}
}
}
