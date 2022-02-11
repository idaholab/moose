//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_common.h"
#include "ADReal.h"

using namespace libMesh;

namespace WallFriction
{

/**
 * Computes Darcy friction factor from Fanning friction factor
 *
 * @param[in] f_F   Fanning friction factor
 */
Real DarcyFrictionFactor(const Real & f_F);
ADReal DarcyFrictionFactor(const ADReal & f_F);

/**
 * Computes Fanning friction factor using Churchill correlation
 *
 * @param Re Reynolds number
 * @param roughness The roughness of the surface
 * @param D_h Hydraulic diameter
 */
Real FanningFrictionFactorChurchill(Real Re, Real roughness, Real D_h);
ADReal FanningFrictionFactorChurchill(ADReal Re, ADReal roughness, ADReal D_h);
}
