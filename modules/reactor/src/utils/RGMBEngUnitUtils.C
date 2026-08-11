//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RGMBEngUnitUtils.h"
#include "CSGPlane.h"
#include "ReactorGeometryMeshBuilderBase.h"

std::vector<std::reference_wrapper<const CSG::CSGSurface>>
RGMBEngUnitUtils::getAxialPlaneSurfaces(CSG::CSGBase & csg_obj,
                                        const std::vector<Real> & axial_boundaries)
{
  std::vector<std::reference_wrapper<const CSG::CSGSurface>> surfaces_by_axial_region;
  Real axial_level = 0.;

  // Check if axial planes have been defined in CSGBase based on a surface name we expect
  // to find
  auto axial_surf_name = RGMB::CSG_AXIAL_PLANE_PREFIX + "0";
  const auto has_axial_surfaces = csg_obj.hasSurface(axial_surf_name);

  for (const auto i : make_range(axial_boundaries.size() + 1))
  {
    axial_surf_name = RGMB::CSG_AXIAL_PLANE_PREFIX + std::to_string(i);
    if (has_axial_surfaces)
      // Surface should exist in CSGBase, retrieve from object
      surfaces_by_axial_region.push_back(csg_obj.getSurfaceByName(axial_surf_name));
    else
    {
      // Surface has not been defined, create it and add to CSGBase
      axial_level += (i != 0) ? axial_boundaries[i - 1] : 0.;
      std::unique_ptr<CSG::CSGSurface> plane_surf_ptr =
          std::make_unique<CSG::CSGPlane>(axial_surf_name, 0, 0, 1, axial_level);
      const auto & plane_surf = csg_obj.addSurface(std::move(plane_surf_ptr));
      surfaces_by_axial_region.push_back(plane_surf);
    }
  }

  return surfaces_by_axial_region;
}
