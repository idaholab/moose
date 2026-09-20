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

CacheInfoThread::CacheInfoThread(MooseMesh & mesh, const bool cache_node_blocks)
  : _mesh(mesh), _cache_node_blocks(cache_node_blocks)
{
}

CacheInfoThread::CacheInfoThread(CacheInfoThread & x, Threads::split /*split*/)
  : _mesh(x._mesh), _cache_node_blocks(x._cache_node_blocks)
{
}

void
CacheInfoThread::cacheNodeBlock(const dof_id_type node_id, const SubdomainID block_id)
{
  if (const auto it = _interface_node_blocks.find(node_id); it != _interface_node_blocks.end())
  {
    it->second.insert(block_id);
    return;
  }

  const auto [it, inserted] = _node_block.emplace(node_id, block_id);
  if (inserted || it->second == block_id)
    return;

  // The elements incident on this node disagree, so it needs the full set from here on
  _interface_node_blocks.emplace(node_id, std::set<SubdomainID>{it->second, block_id});
  _node_block.erase(it);
}

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

    if (_cache_node_blocks)
    {
      const auto block_id = elem->subdomain_id();
      _node_block_ids.insert(block_id);
      for (const auto nd : make_range(elem->n_nodes()))
      {
        const Node & node = *elem->node_ptr(nd);
        cacheNodeBlock(node.id(), block_id);
      }
    }
  }
}

void
CacheInfoThread::join(const CacheInfoThread & y)
{
  // Replaying y's records through cacheNodeBlock() keeps the two containers disjoint: a node that
  // each thread saw with a single, but different, subdomain becomes an interface node here
  for (const auto & [node_id, block_id] : y._node_block)
    cacheNodeBlock(node_id, block_id);
  for (const auto & [node_id, block_ids] : y._interface_node_blocks)
    for (const auto block_id : block_ids)
      cacheNodeBlock(node_id, block_id);
  _node_block_ids.insert(y._node_block_ids.begin(), y._node_block_ids.end());

  _higher_d_elem_side_to_lower_d_elem.insert(y._higher_d_elem_side_to_lower_d_elem.begin(),
                                             y._higher_d_elem_side_to_lower_d_elem.end());
  _lower_d_elem_to_higher_d_elem_side.insert(y._lower_d_elem_to_higher_d_elem_side.begin(),
                                             y._lower_d_elem_to_higher_d_elem_side.end());
  _lower_d_interior_blocks.insert(y._lower_d_interior_blocks.begin(),
                                  y._lower_d_interior_blocks.end());
  _lower_d_boundary_blocks.insert(y._lower_d_boundary_blocks.begin(),
                                  y._lower_d_boundary_blocks.end());
}
