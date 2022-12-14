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
 * Builds the neighborhood information between a heat structure boundary and a flow channel
 */
class FlowChannel3DAlignment
{
public:
  /**
   * @param mesh[in] The mesh
   */
  FlowChannel3DAlignment(const THMMesh & mesh);

  /**
   * Build the neighborhood information between the heat structure boundary and the flow channel
   *
   * @param hs_boundary_info[in] List of tuples (elem_id, side_id) of the hs boundary
   * @param fch_elem_ids[in] List of fch element IDs (i.e. flow channel elements)
   */
  void build(const std::vector<std::tuple<dof_id_type, unsigned short int>> & hs_boundary_info,
             const std::vector<dof_id_type> & fch_elem_ids);

  /**
   * Return the elements IDs associated with the hs side
   */
  const std::vector<dof_id_type> & getHSElementsIDs() const { return _hs_elem_ids; }
  /**
   * Return the boundary info associated with the hs side
   */
  const std::vector<std::tuple<dof_id_type, unsigned short int>> & getHSBoundaryInfo() const
  {
    return _hs_boundary_info;
  }

  /**
   * Return list of flow channel element IDs
   */
  const std::vector<dof_id_type> & getFlowChannelElementIDs() const { return _fch_elem_ids; }

  /**
   * Get the nearest element ID given another element ID
   *
   * @param[in] elem_id Element ID for which we want to obtain the nearest element ID
   */
  const dof_id_type & getNearestElemID(const dof_id_type & elem_id) const;

protected:
  /// Mesh
  const THMMesh & _mesh;
  /// List of hs boundary infos
  std::vector<std::tuple<dof_id_type, unsigned short int>> _hs_boundary_info;
  /// List of hs element IDs
  std::vector<dof_id_type> _hs_elem_ids;
  /// List of fch element IDs
  std::vector<dof_id_type> _fch_elem_ids;
  // Heat structure boundary element centroids
  std::vector<Point> _hs_points;
  // Flow channel element centroids
  std::vector<Point> _fch_points;
  /// Map of the element ID and its nearest element ID
  std::map<dof_id_type, dof_id_type> _nearest_elem_ids;
};
