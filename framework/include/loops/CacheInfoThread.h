//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "MooseTypes.h"

#include "libmesh/elem_range.h"

/**
 * Fills lower-d side maps and node-to-block ownership for MooseMesh::cacheInfo().
 * The range is all elements this rank knows about (local and ghosted).
 */
class CacheInfoThread
{
public:
  /**
   * @param cache_node_blocks Whether to record node-to-block ownership. MooseMesh::cacheInfo()
   * passes false when the relation is constant, i.e. when the mesh has a single subdomain.
   */
  CacheInfoThread(MooseMesh & mesh, bool cache_node_blocks);
  CacheInfoThread(CacheInfoThread & x, Threads::split split);

  void operator()(const ConstElemRange & range);

  void join(const CacheInfoThread & y);

protected:
  /**
   * Record that an element of subdomain \p block_id is incident on the node \p node_id, keeping
   * _node_block and _interface_node_blocks disjoint.
   */
  void cacheNodeBlock(dof_id_type node_id, SubdomainID block_id);

  MooseMesh & _mesh;

  /// Whether node-to-block ownership has to be recorded at all
  const bool _cache_node_blocks;

  /// The subdomain of the elements incident on a node, for the nodes whose incident elements all
  /// agree, which is every node away from a subdomain interface
  std::unordered_map<dof_id_type, SubdomainID> _node_block;

  /// The full subdomain set of the nodes whose incident elements disagree, that is the nodes on a
  /// subdomain interface. A set per node is only affordable because interfaces are a vanishing
  /// fraction of a refined mesh.
  std::map<dof_id_type, std::set<SubdomainID>> _interface_node_blocks;

  /// The subdomains recorded above, so that MooseMesh can intern one singleton set per subdomain
  /// without assuming its own subdomain list is in sync with the elements walked here.
  std::set<SubdomainID> _node_block_ids;

  std::set<SubdomainID> _lower_d_interior_blocks;
  std::set<SubdomainID> _lower_d_boundary_blocks;
  std::unordered_map<std::pair<const Elem *, unsigned short int>, const Elem *>
      _higher_d_elem_side_to_lower_d_elem;
  std::unordered_map<const Elem *, unsigned short int> _lower_d_elem_to_higher_d_elem_side;

  friend class MooseMesh;
};
