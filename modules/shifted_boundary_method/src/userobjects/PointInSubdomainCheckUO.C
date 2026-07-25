//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointInSubdomainCheckUO.h"

#include "libmesh/utility.h"

#include <filesystem>

registerMooseObject("ShiftedBoundaryMethodApp", PointInSubdomainCheckUO);

InputParameters
PointInSubdomainCheckUO::validParams()
{
  InputParameters params = PointInPolyhedronBaseUO::validParams();
  params.addClassDescription(
      "In-Out test with subdomain_id identification based on SurfaceMeshBySubdomainBuilder.");

  params.addRequiredParam<UserObjectName>(
      "builder", "The SurfaceMeshBySubdomainBuilder providing SurfaceElementSets by subdomain_id.");

  return params;
}

PointInSubdomainCheckUO::PointInSubdomainCheckUO(const InputParameters & parameters)
  : PointInPolyhedronBaseUO(parameters),
    _builder(getUserObject<SurfaceMeshBySubdomainBuilder>("builder"))
{
  // Per-subdomain checks use the ray-casting engine over element subsets; the
  // fixed_x_ray (TriangleManifold) backend operates on a whole Tri3 MeshBase and
  // cannot be applied per subdomain.
  if (_method == PointContainmentMethod::FIXED_X_RAY)
    paramError("point_containment_method",
               "fixed_x_ray is not supported by PointInSubdomainCheckUO; use pca_ray or "
               "user_selected_ray.");
}

void
PointInSubdomainCheckUO::initialSetup()
{
  // Map the selected ray backend to the ray direction AdaptiveRayContainmentCheck expects:
  // pca_ray -> (0,0,0) "auto" sentinel; user_selected_ray -> the user's ray_direction.
  const Point ray_direction = (_method == PointContainmentMethod::USER_SELECTED_RAY)
                                  ? _ray_direction
                                  : Point(0.0, 0.0, 0.0);

  // Each subdomain gets its own checker, so the OBB/ray debug meshes must go to
  // distinct files; otherwise later subdomains overwrite earlier ones. Insert the
  // subdomain id before the extension (e.g. "obb.e" -> "obb_2.e"). An empty name
  // means "no debug output" and is left untouched.
  const auto per_subdomain_name = [](const FileName & base, subdomain_id_type id) -> FileName
  {
    if (base.empty())
      return base;
    std::filesystem::path p(base.c_str());
    p.replace_filename(p.stem().string() + "_" + std::to_string(id) + p.extension().string());
    return p.string();
  };

  for (const auto & [subdomain_id, set] : _builder.getSurfaceElementSetsBySubdomain())
  {
    const FileName obb_file = per_subdomain_name(_obb_file_name, subdomain_id);
    const FileName ray_file = per_subdomain_name(_ray_file_name, subdomain_id);
    _subdomain_id_checkers[subdomain_id] =
        std::make_unique<AdaptiveRayContainmentCheck>(set.elements(),
                                                      set.centroids(),
                                                      ray_direction,
                                                      _tolerance,
                                                      _leaf_max_size,
                                                      obb_file,
                                                      ray_file,
                                                      &comm());
  }
}

bool
PointInSubdomainCheckUO::ifInside(const Point & p) const
{
  for (const auto & [_, checker] : _subdomain_id_checkers)
  {
    const SurfaceSide side = checker->sideness(p);
    if (side == SurfaceSide::INSIDE || side == SurfaceSide::ON)
      return true;
  }
  return false;
}

subdomain_id_type
PointInSubdomainCheckUO::whichSubdomain(const Point & p) const
{
  for (const auto & [subdomain_id, checker] : _subdomain_id_checkers)
  {
    const SurfaceSide side = checker->sideness(p);
    if (side == SurfaceSide::INSIDE || side == SurfaceSide::ON)
      return subdomain_id;
  }
  return libMesh::Elem::invalid_subdomain_id;
}
