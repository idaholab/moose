//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MultiAppConservativeTransfer.h"

namespace libMesh
{
class DofObject;
}

/**
 * Copy the value to the target domain from the nearest node in the source domain.
 */
class MultiAppMapNearestNodeTransfer : public MultiAppConservativeTransfer
{
public:
  static InputParameters validParams();

  MultiAppMapNearestNodeTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /**
   * Return the distance between the given point and the farthest corner of the
   * given bounding box.
   * @param p The point to evaluate all distances from.
   * @param bbox The bounding box to evaluate the distance to.
   * @return The maximum distance between the point p and the eight corners of
   * the bounding box bbox.
   */
  Real bboxMaxDistance(const Point & p, const BoundingBox & bbox);

  /**
   * Return the distance between the given point and the nearest corner of the
   * given bounding box.
   * @param p The point to evaluate all distances from.
   * @param bbox The bounding box to evaluate the distance to.
   * @return The minimum distance between the point p and the eight corners of
   * the bounding box bbox.
   */
  Real bboxMinDistance(const Point & p, const BoundingBox & bbox);

  /**
   * Get nearest node candidates.
   * @param local_entities: space locatins and their associated elements
   * @param local_comps: comp num for the unknowns on DofObject. It is useful
   * for higher order method
   */
  void getLocalEntitiesAndComponents(
      MooseMesh * mesh,
      std::vector<std::tuple<Point, DofObject *, BoundaryID>> & local_entities,
      std::vector<unsigned int> & local_comps,
      bool nodal,
      bool constant);

  void getLocalEntities(MooseMesh * mesh,
                        std::vector<std::tuple<Point, DofObject *, BoundaryID>> & local_entities,
                        bool nodal);

  void getBoundaryIDsForCurrProcessor(MooseMesh & master_moose_mesh, std::set<BoundaryID> & curr_bdry_ids);

  std::vector<BoundingBox> getFromBoundingBoxes(std::set<BoundaryID> bdry_ids);

  /// If true then node connections will be cached
  bool _fixed_meshes;

  /// Used to cache nodes
  std::map<dof_id_type, Node *> & _node_map;

  /// Used to cache distances
  std::map<dof_id_type, Real> & _distance_map;

  // These variables allow us to cache nearest node info
  bool & _neighbors_cached;
  std::map<processor_id_type, std::vector<unsigned int>> & _cached_froms;
  std::map<processor_id_type, std::vector<dof_id_type>> & _cached_dof_ids;
  std::map<dof_id_type, unsigned int> & _cached_from_inds;
  std::map<dof_id_type, unsigned int> & _cached_qp_inds;

  std::map<dof_id_type, BoundaryID> _subapp_id_to_bdryid;

private:
  /// Target local nodes for receiving a nodal variable
  std::vector<std::pair<Node *, BoundaryID>> _target_local_nodes;

  /// Extend bounding box by a factor in all directions
  /// Non-zero values of this member may be necessary because the nearest bounding
  /// box does not necessarily give you the closest node/element. It will depend
  /// on the partition and geometry. A node/element will more likely find its
  /// nearest source element/node by extending bounding boxes. If each of the
  /// bounding boxes covers the entire domain, a node/element will be able to
  /// find its nearest source element/node for sure,
  /// but at the same time, more communication will be involved and can be expensive.
  Real _bbox_extend_factor;

  /**
   * Get the local nodes on the target boundary for the transfer
   * @param to_problem_id index of the problem this transfer is sending to
   * @return target local nodes receiving the transferred values
   */
  const std::vector<std::pair<Node *, BoundaryID>> &
  getTargetLocalNodes(const unsigned int to_problem_id);
};
