//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// libMesh includes
#include "libmesh/point.h"

#include "MooseError.h"

/// Maximum number of point neighbors possible
#define MAX_POINT_NEIGHBORS 48

namespace RayTracingCommon
{
/// Identifier for an invalid side index
static const unsigned short invalid_side = static_cast<unsigned short>(-1);
/// Identifier for an invalid edge index
static const unsigned short invalid_edge = static_cast<unsigned short>(-1);
/// Identifier for an invalid vertex index
static const unsigned short invalid_vertex = static_cast<unsigned short>(-1);
/// Identifier for an pair of invalid vertices
static const std::pair<unsigned short, unsigned short> invalid_vertices = {invalid_vertex,
                                                                           invalid_vertex};
/// Identifier for an invalid distance
static const libMesh::Real invalid_distance = -std::numeric_limits<libMesh::Real>::max();
/// Identifier for an invalid point
static const libMesh::Point invalid_point(invalid_distance, invalid_distance, invalid_distance);
}
