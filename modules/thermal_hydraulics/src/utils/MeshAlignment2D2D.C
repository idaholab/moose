//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshAlignment2D2D.h"

MeshAlignment2D2D::MeshAlignment2D2D(const MooseMesh & mesh) : MeshAlignmentOneToMany(mesh) {}

void
MeshAlignment2D2D::initialize(
    const std::vector<std::vector<std::tuple<dof_id_type, unsigned short int>>> & boundary_infos,
    const Point & axis_point,
    const RealVectorValue & axis_direction)
{
  const auto n_boundaries = boundary_infos.size();
  if (n_boundaries > 1)
  {
    extractFromBoundaryInfo(boundary_infos[0],
                            _primary_elem_ids,
                            _primary_side_ids,
                            _primary_elem_points,
                            _primary_node_ids,
                            _primary_node_points);

    for (unsigned int i = 1; i < n_boundaries; ++i)
    {
      std::vector<dof_id_type> elem_ids;
      std::vector<unsigned short int> side_ids;
      std::vector<dof_id_type> node_ids;
      std::vector<Point> elem_points;
      std::vector<Point> node_points;
      extractFromBoundaryInfo(
          boundary_infos[i], elem_ids, side_ids, elem_points, node_ids, node_points);
      _secondary_elem_ids.insert(_secondary_elem_ids.end(), elem_ids.begin(), elem_ids.end());
      _secondary_side_ids.insert(_secondary_side_ids.end(), side_ids.begin(), side_ids.end());
      _secondary_node_ids.insert(_secondary_node_ids.end(), node_ids.begin(), node_ids.end());
      _secondary_elem_points.insert(
          _secondary_elem_points.end(), elem_points.begin(), elem_points.end());
      _secondary_node_points.insert(
          _secondary_node_points.end(), node_points.begin(), node_points.end());
    }

    buildMapping();
    checkAlignment(axis_point, axis_direction);
  }
}
