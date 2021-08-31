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

// Forward declarations
class MultiAppNearestNodeTransfer;
namespace libMesh
{
class DofObject;
}

template <>
InputParameters validParams<MultiAppNearestNodeTransfer>();

/**
 * Copy the value to the target domain from the nearest node in the source domain.
 */
class MultiAppNearestNodeTransfer : public MultiAppConservativeTransfer
{
public:
  static InputParameters validParams();

  MultiAppNearestNodeTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /**
   * Return the nearest node to the point p.
   * @param p The point you want to find the nearest node to.
   * @param distance This will hold the distance between the returned node and p
   * @param mesh The mesh in which we search for the node
   * @param local true if we look at local nodes, otherwise we look at all nodes
   * @return The Node closest to point p.
   */
  Node * getNearestNode(const Point & p, Real & distance, MooseMesh * mesh, bool local);

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
  void getLocalEntitiesAndComponents(MooseMesh * mesh,
                                     std::vector<std::pair<Point, DofObject *>> & local_entities,
                                     std::vector<unsigned int> & local_comps,
                                     bool nodal,
                                     bool constant);

  void getLocalEntities(MooseMesh * mesh,
                        std::vector<std::pair<Point, DofObject *>> & local_entities,
                        bool nodal);

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

private:
  /// Target local nodes for receiving a nodal variable
  std::vector<Node *> _target_local_nodes;

  /// Extend bounding box by a factor in all directions
  /// hat is because the nearest bounding box does not necessarily give
  /// you the closest node/element. It will depend on the partition and geometry.
  /// A node/element will more likely find its nearest source element/node by extending
  /// bounding boxes. If each of the bounding boxes covers the entire domain,
  /// a node/element will be able to find its nearest source element/node for sure,
  /// but at the same time, more communication will be involved and can be expensive.
  Real _bbox_extend_factor;

  /**
   * Get the local nodes on the target boundary for the transfer
   * @param to_problem_id index of the problem this transfer is sending to
   * @return target local nodes receiving the transferred values
   */
  const std::vector<Node *> & getTargetLocalNodes(const unsigned int to_problem_id);
};
