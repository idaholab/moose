//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshSurfaceUtils.h"
#include "MooseUtils.h"

namespace MeshSurfaceUtils
{
libMesh::Point
meshSurfaceCOM(const BoundaryName & boundary, const std::unique_ptr<MeshBase> & mesh)
{
  if (!mesh->is_serial())
    mooseError("findCenterPoint not yet implemented for distributed meshes!");

  libMesh::Point center_point(0, 0, 0);

  BoundaryInfo & mesh_boundary_info = mesh->get_boundary_info();
  boundary_id_type boundary_id = mesh_boundary_info.get_id_by_name(std::string_view(boundary));

  // initialize sums
  double volume_sum = 0;
  Point volume_weighted_centroid_sum(0, 0, 0);

  // loop over all elements in mesh
  for (const auto & elem : mesh->element_ptr_range())
  {
    // loop over all sides in element
    for (const auto side_num : make_range(elem->n_sides()))
    {
      // check if on boundary
      bool on_boundary = mesh_boundary_info.has_boundary_id(elem, side_num, boundary_id);
      if (on_boundary)
      {
        // update running sums
        volume_sum += elem->side_ptr(side_num)->volume();
        volume_weighted_centroid_sum +=
            elem->side_ptr(side_num)->volume() * elem->side_ptr(side_num)->true_centroid();
      }
    }
  }
  center_point = volume_weighted_centroid_sum / volume_sum;
  return center_point;
}

libMesh::Point
meshCOM(const std::unique_ptr<MeshBase> & mesh)
{
  if (!mesh->is_serial())
    mooseError("meshCOM not yet implemented for distributed meshes!");

  libMesh::Point center_point(0, 0, 0);

  // initialize sums
  double volume_sum = 0;
  Point volume_weighted_centroid_sum(0, 0, 0);

  // loop over all elements in mesh
  for (const auto & elem : mesh->element_ptr_range())
  {
    // loop over all sides in element
    for (const auto side_num : make_range(elem->n_sides()))
    {
      // update running sums
      volume_sum += elem->side_ptr(side_num)->volume();
      volume_weighted_centroid_sum +=
          elem->side_ptr(side_num)->volume() * elem->side_ptr(side_num)->true_centroid();
    }
  }
  center_point = volume_weighted_centroid_sum / volume_sum;
  return center_point;
}
}