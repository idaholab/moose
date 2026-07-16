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
RGMBEngUnitUtils::getOuterRadialSurfacesForUnitCell(unsigned int radial_index,
                                                    const std::string & mesh_geometry,
                                                    const std::string & base_name,
                                                    Real halfpitch,
                                                    CSG::CSGBase & csg_obj)
{
  std::vector<std::reference_wrapper<const CSG::CSGSurface>> duct_surfaces;
  const auto n_azim_surfaces = mesh_geometry == "Square" ? 4 : 6;

  // Convert halfpitch to radius (distance from vertex to center)
  const Real angle_offset_degrees = mesh_geometry == "Square" ? 45. : 30.;
  const Real angle_offset_radians = angle_offset_degrees * (M_PI / 180.);
  const auto radius = halfpitch / std::cos(angle_offset_radians);

  Real angle_increment_radians = 360. / n_azim_surfaces * (M_PI / 180.);

  for (const auto i : make_range(n_azim_surfaces))
  {
    const auto surf_name =
        base_name + "_radial_duct_" + std::to_string(radial_index) + "_surf_" + std::to_string(i);

    // Define 3 points on the surface
    const auto current_angle = i * angle_increment_radians + angle_offset_radians;
    const auto next_angle = (i + 1) * angle_increment_radians + angle_offset_radians;
    libMesh::Point p0(radius * std::cos(current_angle), radius * std::sin(current_angle), 0.);
    libMesh::Point p1(radius * std::cos(next_angle), radius * std::sin(next_angle), 0.);
    libMesh::Point p2 = (p0 + p1) / 2.;
    // Place third point above the two others to form a vertical plane
    p2(2) = angle_offset_degrees;

    std::unique_ptr<CSG::CSGSurface> duct_surf_ptr =
        std::make_unique<CSG::CSGPlane>(surf_name, p0, p1, p2);
    const auto & duct_surf = csg_obj.addSurface(std::move(duct_surf_ptr));
    duct_surfaces.push_back(duct_surf);
  }

  return duct_surfaces;
}

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
