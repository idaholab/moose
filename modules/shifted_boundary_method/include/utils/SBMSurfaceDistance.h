//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/point.h"

class SurfaceElement;

namespace SBMUtils
{
/**
 * Returns the vector from pt to the nearest point on the surface element:
 * the normal projection if it falls inside the element, otherwise the
 * nearest edge or vertex.
 *
 * Precondition (mooseAssert in debug builds): the element's sides are EDGE2 or
 * NODEELEM (the only side types this routine handles).
 */
libMesh::Point distanceFrom(const SurfaceElement & surface_elem, const libMesh::Point & pt);
}
