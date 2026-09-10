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
 * Fills subdomain neighbor and attached boundary-id caches for MooseMesh::cacheInfo().
 * The range is active local elements; sides are walked inside operator().
 */
class CacheSubdomainInfoThread
{
public:
  CacheSubdomainInfoThread(MooseMesh & mesh);
  CacheSubdomainInfoThread(CacheSubdomainInfoThread & x, Threads::split split);

  void operator()(const ConstElemRange & range);

  void join(const CacheSubdomainInfoThread & y);

protected:
  MooseMesh & _mesh;

  std::unordered_map<SubdomainID, std::set<SubdomainID>> _neighbor_subs;
  std::unordered_map<SubdomainID, std::set<BoundaryID>> _sub_boundary_ids;
  std::unordered_map<SubdomainID, std::set<BoundaryID>> _neighbor_subdomain_boundary_ids;

  friend class MooseMesh;
};
