//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include "libmesh/point.h"

/**
 * This namespace provides efficient algorithms for quickly hashing different types for checking
 * identity with a very low collision probability.
 */
namespace hashing
{
typedef std::size_t HashValue;

/**
 * Final iteration of the variadic template with no additional arguments
 */
inline void
hashCombine(HashValue & /* seed */)
{
}

/**
 * Variadic template to hashing a combination with finite size
 *   see: https://stackoverflow.com/a/38140932
 */
template <class T, class... Rest>
inline void
hashCombine(HashValue & seed, const T & value, Rest... rest)
{
  std::hash<T> hasher;

  seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  hashCombine(seed, rest...);
}

/**
 * Hash function for sampling 10 points from a large container and hashing
 *   see: https://stackoverflow.com/a/37007715
 */
template <class Container>
HashValue
hashLargeContainer(Container const & container)
{
  std::size_t size = container.size();
  std::size_t stride = 1 + size / 10;
  HashValue seed = size;

  for (std::size_t i = 0; i < size; i += stride)
  {
    hashCombine(seed, container.data()[i]);
  }

  return seed;
}

/**
 * Hashing for Point
 */
inline HashValue
hashCombine(const libMesh::Point & point)
{
  // 'Magic seed' seed that provides entropy against the other hashCombine() seed
  HashValue seed = 3;

  hashCombine(seed, point(0), point(1), point(2));

  return seed;
}

/**
 * Hashing for Point and time, useful for Functions
 */
inline HashValue
hashCombine(Real time, const libMesh::Point & point)
{
  // 'Magic seed' seed that provides entropy against the other hashCombine() seed
  HashValue seed = 42;

  hashCombine(seed, time, point(0), point(1), point(2));

  return seed;
}
}
