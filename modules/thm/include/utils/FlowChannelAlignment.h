#pragma once

#include <map>
#include "libmesh/point.h"
#include "THMMesh.h"

class FlowChannelBase;

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
   * @param flow_channel[in] The flow channel component used for the check
   * @param master_bnd_name[in] The name of the side of 2D component (usually a side set name)
   * @param slave_bnd_name[in] The name of the nodeset of the flow channel
   */
  FlowChannelAlignment(const FlowChannelBase & flow_channel,
                       const BoundaryName & master_bnd_name,
                       const BoundaryName & slave_bnd_name);

  void build();
  bool check() const;

protected:
  /// Flow channel component
  const FlowChannelBase & _flow_channel;
  /// The mesh
  THMMesh & _mesh;
  /// Boundary ID of the master side (2D component)
  BoundaryID _master_bnd_id;
  /// Boundary ID of the slave side (flow channel)
  BoundaryID _slave_bnd_id;

  // master element centroids
  std::vector<Point> _master_points;
  // slave element centroids
  std::vector<Point> _slave_points;
  /// Map of the element ID and its nearest element ID
  std::map<dof_id_type, dof_id_type> _nearest_elem_ids;
  /// Map of the element ID and local side number of the nearest element
  std::map<dof_id_type, unsigned int> _nearest_elem_side;
};
