//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosHeader.h"

namespace Moose::Kokkos
{
namespace Utils
{

/**
 * Returns the sign of a value
 * @param x The value
 * @returns The sign of the value
 */
template <typename T>
KOKKOS_INLINE_FUNCTION T
sign(T x)
{
  return x >= 0.0 ? 1.0 : -1.0;
}

/**
 * Find a value in an array
 * @param target The target value to find
 * @param begin The pointer to the first element of the array
 * @param end The pointer next to the last element of the array
 * @returns The pointer to the target element, \p end if the target element was not found
 */
template <typename T>
KOKKOS_INLINE_FUNCTION const T *
find(const T & target, const T * const begin, const T * const end)
{
  if (begin == end)
    return end;

  auto left = begin;
  auto right = end - 1;

  while (left <= right)
  {
    auto mid = left + (right - left) / 2;

    if (*mid == target)
      return mid;
    else if (*mid < target)
      left = mid + 1;
    else
      right = mid - 1;
  }

  return end;
}

/**
 * Get the index of entry (i, j), j <= i, of a lower-triangular matrix packed row by row
 * @param i The row index
 * @param j The column index
 * @returns The packed index
 */
KOKKOS_INLINE_FUNCTION unsigned int
packedLowerIndex(const unsigned int i, const unsigned int j)
{
  return i * (i + 1) / 2 + j;
}

/**
 * Perform an in-place Cholesky decomposition of a symmetric positive definite matrix
 * @param L The lower triangle of the matrix packed row by row, overwritten by its Cholesky factor
 * @param n The system size
 */
KOKKOS_INLINE_FUNCTION void
choleskyFactor(Real * const L, const unsigned int n)
{
  for (unsigned int i = 0; i < n; ++i)
  {
    for (unsigned int j = 0; j <= i; ++j)
    {
      Real sum = L[packedLowerIndex(i, j)];

      for (unsigned int k = 0; k < j; ++k)
        sum -= L[packedLowerIndex(i, k)] * L[packedLowerIndex(j, k)];

      if (i == j)
        L[packedLowerIndex(i, j)] = ::Kokkos::sqrt(sum);
      else
        L[packedLowerIndex(i, j)] = sum / L[packedLowerIndex(j, j)];
    }
  }
}

/**
 * Solve a linear system with a Cholesky factor from choleskyFactor()
 * The right-hand-side vector is modified after this call
 * @tparam Vector A pointer or a vector view whose operator[] returns a modifiable entry
 * @param L The Cholesky factor packed row by row
 * @param x The solution vector
 * @param b The right-hand-side vector
 * @param n The system size
 */
template <typename Vector>
KOKKOS_INLINE_FUNCTION void
choleskySubstitute(const Real * const L, Vector x, Vector b, const unsigned int n)
{
  for (unsigned int i = 0; i < n; ++i)
  {
    Real sum = b[i];

    for (unsigned int j = 0; j < i; ++j)
      sum -= L[packedLowerIndex(i, j)] * b[j];

    b[i] = sum / L[packedLowerIndex(i, i)];
  }

  for (int i = n - 1; i >= 0; --i)
  {
    Real sum = b[i];

    for (unsigned int j = i + 1; j < n; ++j)
      sum -= L[packedLowerIndex(j, i)] * x[j];

    x[i] = sum / L[packedLowerIndex(i, i)];
  }
}

} // namespace Utils
} // namespace Moose::Kokkos
