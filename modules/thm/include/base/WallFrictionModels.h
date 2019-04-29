#pragma once

#include "libmesh/libmesh_common.h"

using namespace libMesh;

namespace WallFriction
{

/**
 * Computes Darcy friction factor from Fanning friction factor
 *
 * @param[in] f_F   Fanning friction factor
 */
Real DarcyFrictionFactor(const Real & f_F);

/**
 * Computes Fanning friction factor using Churchill correlation
 *
 * @param Re Reynolds number
 * @param roughness The roughness of the surface
 * @param D_h Hydraulic diameter
 */
Real FanningFrictionFactorChurchill(Real Re, Real roughness, Real D_h);
}
