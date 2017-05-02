#ifndef WALLFRICTIONMODELS_H
#define WALLFRICTIONMODELS_H

#include "libmesh/libmesh_common.h"

using namespace libMesh;

namespace WallFriction
{

/**
 * Churchill's friction factor formula
 *
 * @param Re Reynolds number
 * @param roughness The roughness of the surface
 * @param D_h Hydraulic diameter
 */
Real Churchill(Real Re, Real roughness, Real D_h);
}

#endif /* WALLFRICTIONMODELS_H */
