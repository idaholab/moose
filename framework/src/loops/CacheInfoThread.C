//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CacheInfoThread.h"

#include "libmesh/elem.h"
#include "libmesh/threads.h"

namespace
{
template <typename Map>
void
mergeCacheInfoMaps(Map & dest, const Map & src)
{
  for (const auto & [key, values] : src)
    dest[key].insert(values.begin(), values.end());
}
}

CacheInfoThread::CacheInfoThread(MooseMesh & mesh) : _mesh(mesh) {}

CacheInfoThread::CacheInfoThread(CacheInfoThread & x, Threads::split /*split*/) : _mesh(x._mesh) {}

void
CacheInfoThread::operator()(const ConstElemRange & range)
{
  const auto & mesh = _mesh.getMesh();

  for (const auto & elem : range)
  {
    const Elem * ip_elem = elem->interior_parent();

    if (ip_elem)
    {
      unsigned int ip_side = ip_elem->which_side_am_i(elem);

      // For some grid sequencing tests: ip_side == libMesh::invalid_uint
      if (ip_side != libMesh::invalid_uint)
      {
        auto pair = std::make_pair(ip_elem, static_cast<unsigned short int>(ip_side));
        _higher_d_elem_side_to_lower_d_elem.emplace(pair, elem);
        _lower_d_elem_to_higher_d_elem_side.emplace(elem, pair.second);

        auto id = elem->subdomain_id();
        if (ip_elem->neighbor_ptr(ip_side))
        {
          if (mesh.subdomain_name(id).find("INTERNAL_SIDE_LOWERD_SUBDOMAIN_") != std::string::npos)
            _lower_d_interior_blocks.insert(id);
        }
        else
        {
          if (mesh.subdomain_name(id).find("BOUNDARY_SIDE_LOWERD_SUBDOMAIN_") != std::string::npos)
            _lower_d_boundary_blocks.insert(id);
        }
      }
    }

    for (const auto nd : make_range(elem->n_nodes()))
    {
      const Node & node = *elem->node_ptr(nd);
      _block_node_list[node.id()].insert(elem->subdomain_id());
    }
  }
}

void
CacheInfoThread::join(const CacheInfoThread & y)
{
  mergeCacheInfoMaps(_block_node_list, y._block_node_list);
  _higher_d_elem_side_to_lower_d_elem.insert(y._higher_d_elem_side_to_lower_d_elem.begin(),
                                             y._higher_d_elem_side_to_lower_d_elem.end());
  _lower_d_elem_to_higher_d_elem_side.insert(y._lower_d_elem_to_higher_d_elem_side.begin(),
                                             y._lower_d_elem_to_higher_d_elem_side.end());
  _lower_d_interior_blocks.insert(y._lower_d_interior_blocks.begin(),
                                  y._lower_d_interior_blocks.end());
  _lower_d_boundary_blocks.insert(y._lower_d_boundary_blocks.begin(),
                                  y._lower_d_boundary_blocks.end());
}
