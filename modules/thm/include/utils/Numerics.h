#ifndef NUMERICS_H
#define NUMERICS_H

#include "libmesh/libmesh_common.h"

using namespace libMesh;

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


/**
 * Compute Reynolds number
 *
 * @param volume_fraction The volume fraction of the phase
 * @param rho The density of the phase
 * @param v The velocity of the phase
 * @param Dh The hydraulic diameter
 * @param visc The viscosity of the phase
 */
Real Reynolds(Real volume_fraction, Real rho, Real v, Real Dh, Real visc);

/**
 * Computes surface tension [N/m]
 * @param temperature
 * @return returns the surface tension [N/m]
 */
Real surfaceTension(Real temperature);

#endif  // NUMERICS_H
