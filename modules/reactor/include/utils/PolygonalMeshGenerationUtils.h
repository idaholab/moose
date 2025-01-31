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
#include "libmesh/elem.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/dof_object.h"

namespace PolygonalMeshGenerationUtils
{
/**
 * Based on a pair of azimuthal angles, calculates the volume of a TRI6 element with one vertex at
 * the origin, the other two vertices on the unit circle. Here, the second vertex is on the
 * x-axis, the third vertex has an azimuthal angle the summation of the two input angles, and the
 * mid-edge node between the second and third vertices has an azimuthal angle of the first of the
 * two input angles.
 * @param azi_pair a pair of the input azimuthal angles
 * @return the volume of the TRI6 element
 */
Real dummyTRI6VolCalculator(const std::pair<Real, Real> & azi_pair);

/**
 * Makes radial correction to preserve ring area.
 * @param azimuthal_list azimuthal angles (in degrees) of all the nodes on the circle
 * @param full_circle whether the circle is a full or partial circle
 * @param order order of mesh elements
 * @param is_first_value_vertex whether the first value of the azimuthal_list belongs to a vertex
 * instead of a midpoint
 * @return a correction factor to preserve the area of the circle after polygonization during
 * meshing
 */
Real radiusCorrectionFactor(const std::vector<Real> & azimuthal_list,
                            const bool full_circle = true,
                            const unsigned int order = 1,
                            const bool is_first_value_vertex = true);
}
