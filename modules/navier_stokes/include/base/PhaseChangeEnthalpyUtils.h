//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "libmesh/point.h"

#include <algorithm>
#include <cmath>
#include <type_traits>
#include <utility>

/**
 * Small utilities for integrating temperature-dependent heat capacity correlations.
 *
 * Conventions:
 *  - A 1D function of temperature cp(T) is represented with a MOOSE Function object.
 *  - The function is evaluated with Point(T, 0, 0) and the provided time.
 *
 * NOTE: The Function interface returns Real, so these utilities are intended for
 *       computing consistent h(T) / T(h) mappings, not for producing exact AD derivatives.
 */
namespace PhaseChangeEnthalpyUtils
{

namespace detail
{
template <typename T, typename = void>
struct has_time_method : std::false_type
{
};
template <typename T>
struct has_time_method<T, std::void_t<decltype(std::declval<const T &>().time())>> : std::true_type
{
};

template <typename T, typename = void>
struct has_t_method : std::false_type
{
};
template <typename T>
struct has_t_method<T, std::void_t<decltype(std::declval<const T &>().t())>> : std::true_type
{
};

template <typename T, typename = void>
struct has_t_member : std::false_type
{
};
template <typename T>
struct has_t_member<T, std::void_t<decltype(std::declval<const T &>().t)>> : std::true_type
{
};

template <typename T, typename = void>
struct has__t_member : std::false_type
{
};
template <typename T>
struct has__t_member<T, std::void_t<decltype(std::declval<const T &>()._t)>> : std::true_type
{
};
} // namespace detail

/**
 * Best-effort extraction of the simulation time from a state argument.
 *
 * Different MOOSE versions have used different APIs for accessing time on the
 * state argument passed into functors. This helper uses compile-time detection
 * to support the common variants without introducing hard version dependencies.
 *
 * Falls back to 0.0 if no known accessor is available.
 */
template <typename StateT>
inline Real
timeFromStateArg(const StateT & state)
{
  if constexpr (detail::has_time_method<StateT>::value)
    return state.time();
  else if constexpr (detail::has_t_method<StateT>::value)
    return state.t();
  else if constexpr (detail::has_t_member<StateT>::value)
    return state.t;
  else if constexpr (detail::has__t_member<StateT>::value)
    return state._t;
  else
    return 0.0;
}

inline Real
evalTemperatureFunction(const Function & f, const Real time, const Real T)
{
  const libMesh::Point p(T, 0.0, 0.0);
  return f.value(time, p);
}

/**
 * Composite Simpson integration of a temperature function over [T0, T1].
 * If T1 < T0, the integral sign is handled automatically.
 */
inline Real
integrateTemperatureFunctionSimpson(const Function & f,
                                    const Real time,
                                    Real T0,
                                    Real T1,
                                    unsigned int n_subintervals)
{
  if (T0 == T1)
    return 0.0;

  Real a = T0;
  Real b = T1;
  Real sign = 1.0;
  if (b < a)
  {
    std::swap(a, b);
    sign = -1.0;
  }

  unsigned int n = std::max(2u, n_subintervals);
  // Simpson requires an even number of subintervals
  if (n % 2 == 1)
    ++n;

  const Real h = (b - a) / n;

  Real sum = evalTemperatureFunction(f, time, a) + evalTemperatureFunction(f, time, b);

  for (unsigned int i = 1; i < n; ++i)
  {
    const Real x = a + i * h;
    sum += (i % 2 ? 4.0 : 2.0) * evalTemperatureFunction(f, time, x);
  }

  return sign * sum * h / 3.0;
}

/**
 * Invert I(T) = h_target for I(T) = \int_{T_ref}^{T} cp(T') dT' using bracketing + bisection.
 *
 * Assumes cp(T) >= 0 and monotonicity of I(T).
 * If bracketing fails, the last expanded bound is returned (still monotone and safe).
 */
inline Real
invertIntegratedCpBisection(const Function & cp,
                            const Real time,
                            const Real T_ref,
                            const Real h_target,
                            const unsigned int n_subintervals,
                            const unsigned int max_its,
                            const Real rel_tol,
                            const Real abs_tol)
{
  const Real tol = std::max(abs_tol, rel_tol * std::abs(h_target));
  if (std::abs(h_target) <= tol)
    return T_ref;

  // Bracket search
  Real T_low = T_ref;
  Real T_high = T_ref;
  Real I_low = 0.0;
  Real I_high = 0.0;

  constexpr unsigned int max_expand = 60;
  Real step = 1.0;

  if (h_target > 0.0)
  {
    // search upward
    T_low = T_ref;
    I_low = 0.0;
    T_high = T_ref + step;
    I_high = integrateTemperatureFunctionSimpson(cp, time, T_ref, T_high, n_subintervals);

    unsigned int expand = 0;
    while (I_high < h_target && expand < max_expand)
    {
      step *= 2.0;
      T_high = T_ref + step;
      I_high = integrateTemperatureFunctionSimpson(cp, time, T_ref, T_high, n_subintervals);
      ++expand;
    }

    if (I_high < h_target)
      return T_high; // could not bracket
  }
  else
  {
    // search downward
    T_high = T_ref;
    I_high = 0.0;
    T_low = T_ref - step;
    I_low = integrateTemperatureFunctionSimpson(cp, time, T_ref, T_low, n_subintervals);

    unsigned int expand = 0;
    while (I_low > h_target && expand < max_expand)
    {
      step *= 2.0;
      T_low = T_ref - step;
      I_low = integrateTemperatureFunctionSimpson(cp, time, T_ref, T_low, n_subintervals);
      ++expand;
    }

    if (I_low > h_target)
      return T_low; // could not bracket
  }

  // Bisection
  for (unsigned int it = 0; it < max_its; ++it)
  {
    const Real T_mid = 0.5 * (T_low + T_high);
    const Real I_mid = integrateTemperatureFunctionSimpson(cp, time, T_ref, T_mid, n_subintervals);

    const Real F_mid = I_mid - h_target;
    if (std::abs(F_mid) <= tol)
      return T_mid;

    // Monotone update
    if (h_target > 0.0)
    {
      if (I_mid < h_target)
        T_low = T_mid;
      else
        T_high = T_mid;
    }
    else
    {
      if (I_mid > h_target)
        T_high = T_mid;
      else
        T_low = T_mid;
    }
  }

  return 0.5 * (T_low + T_high);
}

} // namespace PhaseChangeEnthalpyUtils
