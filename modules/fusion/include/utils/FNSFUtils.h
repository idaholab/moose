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
#include "libmesh/point.h"

using namespace libMesh;

namespace FNSF
{
extern const Real R0;  // Major radius, m
extern const Real a;   // Minor radius, m
extern const Real tau; // triangularity
extern const Real k;   // elongation
extern const Real b;

/**
 * Parametric definition of plasma shape (x-z plane cross-section)
 */
Point torus(Real xi);
/**
 * Orthogonal vector
 */
Point orthogonal(Real xi);

/**
 * Convert (r, z) coordinates to (xi, depth)
 */
std::pair<Real, Real> find_xi_depth(Real r, Real z);

} // namespace FNSF
