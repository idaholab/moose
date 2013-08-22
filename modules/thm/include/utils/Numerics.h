#ifndef NUMERICS_H
#define NUMERICS_H

#include "libmesh/libmesh_common.h"

namespace Numerics {
  extern void* auxpt;

Real Newton_Solver(Real x0, Real f0, Real (*fct)(Real x, Real f0), Real (*dfct)(Real x), Real tola, Real tolr, unsigned int n_max);
}


template <typename T>
int sgn(T val)
{
  return (T(0) < val) - (val < T(0));
}

#endif  // NUMERICS_H
