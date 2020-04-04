//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"

/**
 * Class designed to hold node ID information and information
 * about nodal connectivity.
 * Node ID is either:
 * - global, meaning that it is the number assigned to the node in the mesh
 * - sequential, meaning that it is the number used within this class
 * The sequential numbering runs from zero to number_of_nodes - 1
 *
 * The use case of interest in PorousFlow is where many quantities are
 * recorded at each node in the mesh.  To record this, a std::vector<quantity>
 * may be used.  But if the global node IDs are sparsely distributed between
 * zero and the maximum node number, then basing the std::vector on
 * global ID is inefficient because there are many elements in the
 * std::vector that are unused.  Instead, a std::vector based on
 * the sequential node ID may be used, which is optimally memory efficient.
 * (A std::map<dof_id_type, quantity> may be used, but it is slow.)
 *
 * Nodal connectivity is defined between two global node IDs.  Note
 * that it is uni-directional, meaning that if node 12 is connected to
 * node 321, then node 321 is not necessarily connected to node 12
 * (the user must explicitly add both the 12->321 and 321->12 connections if that is desired).
 *
 * This class is designed to be used as follows:
 * (1) instantiation
 * (2) populate the nodal information using addGlobalNode
 * (3) finalizeAddingGlobalNodes()
 * (4) populate the connectivity using addConnection
 * (5) finalizeAddingConnections()
 * (6) functions like sequentialID and sequentialNeighborsOfGlobalID are now ready for use
 */
class PorousFlowConnectedNodes
{
public:
  PorousFlowConnectedNodes();

  /// clear all data in readiness for adding global nodes and connections
  void clear();

  /**
   * Add the given global_node_ID to the internal data structures
   * If the global node ID has already been added this method does nothing
   * @param global_node_ID node number in the mesh
   */
  void addGlobalNode(dof_id_type global_node_ID);

  /// Signal that all global node IDs have been added to the internal data structures
  void finalizeAddingGlobalNodes();

  /// number of nodes known by this class
  std::size_t numNodes() const;

  /**
   * Return the global node ID (node number in the mesh) corresponding to the provided sequential
   * node ID
   * @param sequential_node_ID the sequential node ID
   */
  dof_id_type globalID(dof_id_type sequential_node_ID) const;

  /**
   * Return the sequential node ID corresponding to the global node ID
   * This is guaranteed to lie in the range [0, numNodes() - 1]
   * @param global_node_ID global node ID (ie the node number in the mesh)
   */
  dof_id_type sequentialID(dof_id_type global_node_ID) const;

  /// Vector of all global node IDs (node numbers in the mesh)
  const std::vector<dof_id_type> & globalIDs() const;

  /**
   * Specifies that global_node_to is connected to global_node_from.
   * Hence, globalConnectionsToGlobalID(global_node_from) will contain global_node_to.
   * If the connection has already been added this method does nothing
   * Recall that connections are uni-directional, so if you desire bi-directional
   * connectivity you must call addConnection twice (the second time with arguments swapped)
   * @param global_node_from global node ID of the 'from' node
   * @param global_node_to global node ID of the 'to' node
   */
  void addConnection(dof_id_type global_node_from, dof_id_type global_node_to);

  /// Signal that all global node IDs have been added to the internal data structures
  void finalizeAddingConnections();

  /**
   * Return all the nodes (sequential node IDs) connected to the given global node ID
   * All elements of the returned vector are guaranteed to be unique and lie in the range [0,
   * numNodes() - 1]
   */
  const std::vector<dof_id_type> &
  sequentialConnectionsToGlobalID(dof_id_type global_node_ID) const;

  /**
   * Return all the nodes (sequential node IDs) connected to the given sequential node ID
   * All elements of the returned vector are guaranteed to be unique and lie in the range [0,
   * numNodes() - 1]
   */
  const std::vector<dof_id_type> &
  sequentialConnectionsToSequentialID(dof_id_type sequential_node_ID) const;

  /// Return all the nodes (global node IDs) connected to the given global node ID
  const std::vector<dof_id_type> & globalConnectionsToGlobalID(dof_id_type global_node_ID) const;

  /// Return all the nodes (global node IDs) connected to the given sequential node ID
  const std::vector<dof_id_type> &
  globalConnectionsToSequentialID(dof_id_type sequential_node_ID) const;

  /// Return the index of global_node_ID_to in the globalConnectionsToGlobalID(global_node_ID_from) vector
  unsigned indexOfGlobalConnection(dof_id_type global_node_ID_from,
                                   dof_id_type global_node_ID_to) const;

  /// Return the index of sequential_node_ID_to in the sequentialConnectionsToSequentialID(sequential_node_ID_from) vector
  unsigned indexOfSequentialConnection(dof_id_type sequential_node_ID_from,
                                       dof_id_type sequential_node_ID_to) const;

  /**
   * Return the size of _sequential_id, for checking memory efficiency.
   * The memory wasted by this class is (sizeSequential() - numNodes()) * (size of dof_id_type).
   * finalizeAddingGlobalNodes() must have been called prior to calling this method
   */
  std::size_t sizeSequential() const;

private:
  bool _still_adding_global_nodes;
  dof_id_type _min_global_id;
  dof_id_type _max_global_id;
  std::set<dof_id_type> _set_of_global_ids;
  std::vector<dof_id_type> _global_id;
  std::vector<dof_id_type> _sequential_id;

  bool _still_adding_connections;
  std::vector<std::set<dof_id_type>> _neighbor_sets;
  std::vector<std::vector<dof_id_type>> _sequential_neighbors;
  std::vector<std::vector<dof_id_type>> _global_neighbors;
};
