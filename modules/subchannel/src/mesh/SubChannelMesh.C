//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubChannelMesh.h"

InputParameters
SubChannelMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.addClassDescription("Base class for all mesh containers");
  return params;
}

SubChannelMesh::SubChannelMesh(const InputParameters & params) : MooseMesh(params), _kij(0.0) {}

SubChannelMesh::SubChannelMesh(const SubChannelMesh & other_mesh)
  : MooseMesh(other_mesh),
    _pin_mesh_exist(other_mesh._pin_mesh_exist),
    _duct_mesh_exist(other_mesh._duct_mesh_exist),
    _unheated_length_entry(other_mesh._unheated_length_entry),
    _heated_length(other_mesh._heated_length),
    _unheated_length_exit(other_mesh._unheated_length_exit),
    _z_grid(other_mesh._z_grid),
    _k_grid(other_mesh._k_grid),
    _duct_nodes(other_mesh._duct_nodes),
    _chan_to_duct_node_map(other_mesh._chan_to_duct_node_map),
    _duct_node_to_chan_map(other_mesh._duct_node_to_chan_map),
    _spacer_z(other_mesh._spacer_z),
    _spacer_k(other_mesh._spacer_k),
    _z_blockage(other_mesh._z_blockage),
    _index_blockage(other_mesh._index_blockage),
    _reduction_blockage(other_mesh._reduction_blockage),
    _kij(other_mesh._kij),
    _pitch(other_mesh._pitch),
    _pin_diameter(other_mesh._pin_diameter),
    _n_cells(other_mesh._n_cells)
{
  _subchannel_position = other_mesh._subchannel_position;
}

void
SubChannelMesh::generateZGrid(Real unheated_length_entry,
                              Real heated_length,
                              Real unheated_length_exit,
                              unsigned int n_cells,
                              std::vector<Real> & z_grid)
{
  Real L = unheated_length_entry + heated_length + unheated_length_exit;
  Real dz = L / n_cells;
  for (unsigned int i = 0; i < n_cells + 1; i++)
    z_grid.push_back(dz * i);
}

unsigned int
SubChannelMesh::getZIndex(const Point & point) const
{
  if (_z_grid.size() == 0)
    mooseError("_z_grid is empty.");

  if (point(2) <= _z_grid[0])
    return 0;
  if (point(2) >= _z_grid[_z_grid.size() - 1])
    return _z_grid.size() - 1;

  unsigned int lo = 0;
  unsigned int hi = _z_grid.size();
  while (lo < hi)
  {
    unsigned int mid = (lo + hi) / 2;
    if (std::abs(_z_grid[mid] - point(2)) < 1e-5)
      return mid;
    else if (_z_grid[mid] < point(2))
      lo = mid;
    else
      hi = mid;
  }
  return lo;
}

Node *
SubChannelMesh::getDuctNodeFromChannel(Node * channel_node) const
{
  auto it = _chan_to_duct_node_map.find(channel_node);
  return (it == _chan_to_duct_node_map.end()) ? nullptr : it->second;
}

Node *
SubChannelMesh::getChannelNodeFromDuct(Node * duct_node) const
{
  auto it = _duct_node_to_chan_map.find(duct_node);
  return (it == _duct_node_to_chan_map.end()) ? nullptr : it->second;
}

void
SubChannelMesh::setChannelToDuctMaps(const std::vector<Node *> & duct_nodes)
{
  _duct_nodes.clear();
  _chan_to_duct_node_map.clear();
  _duct_node_to_chan_map.clear();

  if (_z_grid.empty())
    mooseError("setChannelToDuctMaps: _z_grid is empty; cannot match duct nodes by z.");

  if (_subchannel_position.empty())
    mooseError(
        "setChannelToDuctMaps: _subchannel_position is empty; cannot map duct nodes to channels.");

  for (size_t i = 0; i < duct_nodes.size(); ++i)
  {
    Node * dn = duct_nodes[i];

    // 1) Find closest subchannel center in XY
    unsigned int min_chan = 0;
    Real min_dist = std::numeric_limits<Real>::max();

    const Point ductpos((*dn)(0), (*dn)(1), 0.0);

    for (unsigned int j = 0; j < _subchannel_position.size(); ++j)
    {
      const Point chanpos(_subchannel_position[j][0], _subchannel_position[j][1], 0.0);
      const Real dist = (chanpos - ductpos).norm();

      if (dist < min_dist)
      {
        min_dist = dist;
        min_chan = j;
      }
    }

    // 2) Find channel node at the same z using getZIndex + virtual accessor
    const unsigned int iz = getZIndex(*dn);
    Node * chan_node = getChannelNode(min_chan, iz);

    // 3) Store bidirectional mapping
    _duct_node_to_chan_map[dn] = chan_node;
    _chan_to_duct_node_map[chan_node] = dn;
  }

  _duct_nodes = duct_nodes;
  _duct_mesh_exist = true;
}
