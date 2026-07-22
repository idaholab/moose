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

registerMooseObject("ShiftedBoundaryMethodApp", PointInSubdomainCheckUO);

InputParameters
PointInSubdomainCheckUO::validParams()
{
  InputParameters params = PointInPolyhedronBaseUO::validParams();
  params.addClassDescription(
      "In-Out test with subdomain_id identification based on SurfaceMeshBySubdomainBuilder.");

  params.addRequiredParam<UserObjectName>(
      "builder", "The SurfaceMeshBySubdomainBuilder providing boundary elements by subdomain_id.");

  return params;
}

PointInSubdomainCheckUO::PointInSubdomainCheckUO(const InputParameters & parameters)
  : PointInPolyhedronBaseUO(parameters),
    _builder(getUserObject<SurfaceMeshBySubdomainBuilder>("builder"))
{
}

void
PointInSubdomainCheckUO::initialSetup()
{
  const auto & bnd_elems_by_subdomain_id = _builder.getBoundaryElementsBySubdomain();
  const auto & centroids_by_subdomain_id = _builder.getCentroidsBySubdomain();

  for (const auto & [subdomain_id, elements] : bnd_elems_by_subdomain_id)
  {
    auto checker = std::make_unique<PointInPolyhedronCheck>(
        elements,
        libmesh_map_find(centroids_by_subdomain_id, subdomain_id),
        _ray_direction,
        _brute_force,
        _eps,
        _leaf_max_size,
        _obb_file_name,
        _ray_file_name);

    _subdomain_id_checkers[subdomain_id] = std::move(checker);
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
