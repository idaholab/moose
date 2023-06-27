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

/**
 * Computes Fanning friction factor using Cheng-Todreas correlation
 *
 * @param Re   Reynolds number
 * @param a    Correlation constant
 * @param b    Correlation constant
 * @param c    Correlation constant
 * @param n    Correlation constant
 * @param PoD  Pitch-to-diameter ratio
 */

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
auto
FanningFrictionFactorCheng(
    const T1 & Re, const T2 & a, const T3 & b, const T4 & c, const T5 & n, const T6 & PoD)
{
  return (a + b * (PoD - 1) + c * std::pow(PoD - 1, 2)) / std::pow(std::max(Re, 10.0), n);
}
}
