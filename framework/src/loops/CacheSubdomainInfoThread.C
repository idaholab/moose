//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CacheSubdomainInfoThread.h"

#include "libmesh/elem.h"
#include "libmesh/threads.h"

namespace
{
template <typename Map>
void
mergeSubdomainInfoMaps(Map & dest, const Map & src)
{
  for (const auto & [key, values] : src)
    dest[key].insert(values.begin(), values.end());
}
}

CacheSubdomainInfoThread::CacheSubdomainInfoThread(MooseMesh & mesh) : _mesh(mesh) {}

CacheSubdomainInfoThread::CacheSubdomainInfoThread(CacheSubdomainInfoThread & x,
                                                   Threads::split /*split*/)
  : _mesh(x._mesh)
{
}

void
CacheSubdomainInfoThread::operator()(const ConstElemRange & range)
{
  for (const auto & elem : range)
  {
    SubdomainID subdomain_id = elem->subdomain_id();
    const auto elem_boundary_ids = _mesh.getBoundaryIDs(elem);
    for (const auto side : make_range(elem->n_sides()))
    {
      const auto & boundary_ids = elem_boundary_ids[side];
      _sub_boundary_ids[subdomain_id].insert(boundary_ids.begin(), boundary_ids.end());

      const Elem * neig = elem->neighbor_ptr(side);
      if (neig)
      {
        _neighbor_subdomain_boundary_ids[neig->subdomain_id()].insert(boundary_ids.begin(),
                                                                      boundary_ids.end());
        SubdomainID neighbor_subdomain_id = neig->subdomain_id();
        if (neighbor_subdomain_id != subdomain_id)
          _neighbor_subs[subdomain_id].insert(neighbor_subdomain_id);
      }
    }
  }
}

void
CacheSubdomainInfoThread::join(const CacheSubdomainInfoThread & y)
{
  mergeSubdomainInfoMaps(_neighbor_subs, y._neighbor_subs);
  mergeSubdomainInfoMaps(_sub_boundary_ids, y._sub_boundary_ids);
  mergeSubdomainInfoMaps(_neighbor_subdomain_boundary_ids, y._neighbor_subdomain_boundary_ids);
}
