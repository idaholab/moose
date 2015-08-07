#ifndef NUMERICS_H
#define NUMERICS_H

#include "libmesh/libmesh_common.h"

/**
 * The sign function
 * @param val The argument of the sign function
 * @return -1 for negative values, 0 for zero and 1 for positive values
 */
template <typename T>
int sgn(T val)
{
  return (T(0) < val) - (val < T(0));
}

#endif  // NUMERICS_H
