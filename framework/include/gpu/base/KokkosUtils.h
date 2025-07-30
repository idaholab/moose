//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUHeader.h"

namespace Moose
{
namespace Kokkos
{
namespace Utils
{

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

} // namespace Utils
} // namespace Kokkos
} // namespace Moose
