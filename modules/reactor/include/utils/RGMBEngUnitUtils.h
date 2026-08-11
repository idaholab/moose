//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGBase.h"

namespace RGMBEngUnitUtils
{

/**
 * Get CSGSurfaces corresponding to axial planes of the extruded RGMB mesh
 * @param csg_obj Reference to CSGBase object for adding defined surfaces to
 * @param axial_boundaries List of axial boundaries of the extruded mesh
 * @return vector of surfaces that correspond to axial planes of extruded mesh
 */
std::vector<std::reference_wrapper<const CSG::CSGSurface>>
getAxialPlaneSurfaces(CSG::CSGBase & csg_obj, const std::vector<Real> & axial_boundaries);

}
