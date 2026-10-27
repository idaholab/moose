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
  CacheInfoThread(MooseMesh & mesh);
  CacheInfoThread(CacheInfoThread & x, Threads::split split);

  void operator()(const ConstElemRange & range);

  void join(const CacheInfoThread & y);

protected:
  MooseMesh & _mesh;

  std::map<dof_id_type, std::set<SubdomainID>> _block_node_list;
  std::set<SubdomainID> _lower_d_interior_blocks;
  std::set<SubdomainID> _lower_d_boundary_blocks;
  std::unordered_map<std::pair<const Elem *, unsigned short int>, const Elem *>
      _higher_d_elem_side_to_lower_d_elem;
  std::unordered_map<const Elem *, unsigned short int> _lower_d_elem_to_higher_d_elem_side;

  friend class MooseMesh;
};
