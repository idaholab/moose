//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
 * Compute the Cholesky decomposition of a matrix in place
 *
 * The lower triangle is overwritten by the factor. A matrix that is not positive definite yields a
 * non-positive pivot, which the return value reports rather than propagating a division by zero, so
 * that a caller factoring many matrices can fall back for the ones that fail.
 *
 * @param A The row-major matrix, overwritten by its factor
 * @param n The system size
 * @returns Whether the matrix factored, which is to say every pivot was positive
 */
KOKKOS_INLINE_FUNCTION bool
choleskyFactor(Real * const A, const unsigned int n)
{
  for (unsigned int i = 0; i < n; ++i)
  {
    for (unsigned int j = 0; j <= i; ++j)
    {
      Real sum = A[j + n * i];

      for (unsigned int k = 0; k < j; ++k)
        sum -= A[k + n * i] * A[k + n * j];

      if (i == j)
      {
        if (sum <= 0)
          return false;

        A[j + n * i] = ::Kokkos::sqrt(sum);
      }
      else
        A[j + n * i] = sum / A[j + n * j];
    }
  }

  return true;
}

/**
 * Solve with an already computed Cholesky factor by forward and back substitution
 *
 * The factor is read and left intact, so one factorization serves any number of right-hand sides,
 * which is what lets a factorization be reused across the applications of a preconditioner.
 *
 * @param A The row-major Cholesky factor, as choleskyFactor() leaves it
 * @param x The solution vector
 * @param b The right-hand-side vector, modified after this call
 * @param n The system size
 */
KOKKOS_INLINE_FUNCTION void
choleskySubstitute(const Real * const A, Real * const x, Real * const b, const unsigned int n)
{
  for (unsigned int i = 0; i < n; ++i)
  {
    Real sum = b[i];

    for (unsigned int j = 0; j < i; ++j)
      sum -= A[j + n * i] * b[j];

    b[i] = sum / A[i + n * i];
  }

  for (int i = n - 1; i >= 0; --i)
  {
    Real sum = b[i];

    for (unsigned int j = i + 1; j < n; ++j)
      sum -= A[i + n * j] * x[j];

    x[i] = sum / A[i + n * i];
  }
}

/**
 * Perform an in-place linear solve using Cholesky decomposition
 * Matrix and right-hand-side vector are modified after this call
 * @param A The row-major matrix
 * @param x The solution vector
 * @param b The right-hand-side vector
 * @param n The system size
 */
KOKKOS_INLINE_FUNCTION void
choleskySolve(Real * const A, Real * const x, Real * const b, const unsigned int n)
{
  choleskyFactor(A, n);
  choleskySubstitute(A, x, b, n);
}

} // namespace Utils
} // namespace Moose::Kokkos
