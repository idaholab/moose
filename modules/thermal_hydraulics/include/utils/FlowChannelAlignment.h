//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <map>
#include "libmesh/point.h"
#include "THMMesh.h"

/**
 * Checks alignment of 1D component with 2D one
 *
 * Typically used to make sure that spatial discretization of a flow channel matches
 * the discretization of a heat structure
 */
class FlowChannelAlignment
{
public:
  /**
   * @param mesh[in] The mesh
   */
  FlowChannelAlignment(const THMMesh & mesh);

  /**
   * Build the neighborhood information between master and slave side
   *
   * @param master_boundary_info[in] List of tuples (elem_id, side_id) of the master side (i.e. heat
   * structure side)
   * @param slave_elem_ids[in] List of slave element IDs (i.e. flow channel elements)
   */
  void build(const std::vector<std::tuple<dof_id_type, unsigned short int>> & master_boundary_info,
             const std::vector<dof_id_type> & slave_elem_ids);

  /**
   * Check the alignment of the flow channel elements with the heat structure elements
   *
   * @param slave_elem_ids[in] The vector of flow channel element IDs
   */
  bool check(const std::vector<dof_id_type> & slave_elem_ids) const;

  /**
   * Return the boundary info associated with the master side
   */
  const std::vector<std::tuple<dof_id_type, unsigned short int>> & getMasterBoundaryInfo() const
  {
    return _master_boundary_info;
  }

  /**
   * Return list of slave side element IDs
   */
  const std::vector<dof_id_type> & getSlaveElementIDs() const { return _slave_elem_ids; }

  /**
   * Get the nearest element ID given another element ID
   *
   * @param[in] elem_id Element ID for which we want to obtain the nearest element ID
   */
  const dof_id_type & getNearestElemID(const dof_id_type & elem_id) const;

protected:
  /// Mesh
  const THMMesh & _mesh;
  /// List of master element boundary infos
  std::vector<std::tuple<dof_id_type, unsigned short int>> _master_boundary_info;
  /// List of slave element IDs
  std::vector<dof_id_type> _slave_elem_ids;
  // master element centroids
  std::vector<Point> _master_points;
  // slave element centroids
  std::vector<Point> _slave_points;
  /// Map of the element ID and its nearest element ID
  std::map<dof_id_type, dof_id_type> _nearest_elem_ids;
  /// Map of the element ID and local side number of the nearest element
  std::map<dof_id_type, unsigned int> _nearest_elem_side;
};
