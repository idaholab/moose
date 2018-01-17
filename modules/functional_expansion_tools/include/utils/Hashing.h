/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef HASHING_H
#define HASHING_H

// MOOSE includes
#include "MooseTypes.h"

namespace hashing
{
typedef std::size_t HashValue;

/// Final iteration of the variadic template with no additional arguments
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
  HashValue value = size;

  for (std::size_t i = 0; i < size; i += stride)
  {
    hashCombine(value, container.data()[i]);
  }

  return value;
}

/// Hashing for Point
inline HashValue
hashCombine(const libMesh::Point & point)
{
  HashValue value = 3;

  hashCombine(value, point(0), point(1), point(2));

  return value;
}

/// Hashing for Point and time, useful for Functions
inline HashValue
hashCombine(Real time, const libMesh::Point & point)
{
  HashValue value = 4;

  hashCombine(value, time, point(0), point(1), point(2));

  return value;
}
}

#endif // HASHING_H
