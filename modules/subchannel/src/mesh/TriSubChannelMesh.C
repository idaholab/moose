/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "TriSubChannelMesh.h"
#include <cmath>
#include "libmesh/node.h"

registerMooseObject("SubChannelApp", TriSubChannelMesh);

InputParameters
TriSubChannelMesh::validParams()
{
  InputParameters params = SubChannelMesh::validParams();
  return params;
}

TriSubChannelMesh::TriSubChannelMesh(const InputParameters & params)
  : SubChannelMesh(params), _pin_mesh_exist(false), _duct_mesh_exist(false)
{
}

TriSubChannelMesh::TriSubChannelMesh(const TriSubChannelMesh & other_mesh)
  : SubChannelMesh(other_mesh),
    _subchannel_position(other_mesh._subchannel_position),
    _n_rings(other_mesh._n_rings),
    _n_channels(other_mesh._n_channels),
    _flat_to_flat(other_mesh._flat_to_flat),
    _dwire(other_mesh._dwire),
    _hwire(other_mesh._hwire),
    _duct_to_rod_gap(other_mesh._duct_to_rod_gap),
    _nodes(other_mesh._nodes),
    _duct_nodes(other_mesh._duct_nodes),
    _chan_to_duct_node_map(other_mesh._chan_to_duct_node_map),
    _duct_node_to_chan_map(other_mesh._duct_node_to_chan_map),
    _gap_to_chan_map(other_mesh._gap_to_chan_map),
    _chan_to_gap_map(other_mesh._chan_to_gap_map),
    _sign_id_crossflow_map(other_mesh._sign_id_crossflow_map),
    _gij_map(other_mesh._gij_map),
    _rod_position(other_mesh._rod_position),
    _rods_in_rings(other_mesh._rods_in_rings),
    _subchannel_to_rod_map(other_mesh._subchannel_to_rod_map),
    _gap_to_rod_map(other_mesh._gap_to_rod_map),
    _nrods(other_mesh._nrods),
    _n_gaps(other_mesh._n_gaps),
    _subch_type(other_mesh._subch_type),
    _gap_type(other_mesh._gap_type),
    _gap_pairs_sf(other_mesh._gap_pairs_sf),
    _chan_pairs_sf(other_mesh._chan_pairs_sf),
    _pin_to_chan_map(other_mesh._pin_to_chan_map),
    _pin_mesh_exist(other_mesh._pin_mesh_exist),
    _duct_mesh_exist(other_mesh._duct_mesh_exist)
{
}

std::unique_ptr<MooseMesh>
TriSubChannelMesh::safeClone() const
{
  return libmesh_make_unique<TriSubChannelMesh>(*this);
}

unsigned int
TriSubChannelMesh::getSubchannelIndexFromPoint(const Point & p) const
{
  /// Function that returns the subchannel index given a point
  return this->channelIndex(p);
}

unsigned int
TriSubChannelMesh::channelIndex(const Point & p) const
{
  /// Function that returns the subchannel index given a point
  /// Determining a channel index given a point
  /// Looping over all subchannels to determine the closest one to the point
  /// Special treatment for edge and corner subchannels since deformed elements may lead to wrong transfers

  // Distances to determine the closest subchannel
  Real distance0 = 1.0e+8; // Dummy distance to keep updating the closes distance found
  Real distance1;          // Distance to be updated with the current subchannel distance
  unsigned int j = 0;      // Index to keep track of the closest point to subchannel

  // Projecting point into hexahedral coordinated to determine if the point belongs to a center
  // subchannel
  Real distance_outer_ring = _flat_to_flat / 2 - _duct_to_rod_gap - _rod_diameter / 2;
  Real channel_distance = std::sqrt(std::pow(p(0), 2) + std::pow(p(1), 2));
  Real angle = std::abs(std::atan(p(1) / p(0)));
  Real projection_angle =
      angle - libMesh::pi / 6 - std::trunc(angle / (libMesh::pi / 3)) * (libMesh::pi / 3);
  channel_distance = channel_distance * std::cos(projection_angle);

  // Projecting point on top edge to determine if the point is a corner or edge subchannel by x
  // coordinate
  Real loc_angle = std::atan(p(1) / p(0));
  if (p(0) <= 0)
    loc_angle += libMesh::pi;
  else if (p(0) >= 0 && p(1) <= 0)
    loc_angle += 2 * libMesh::pi;
  Real rem_ang = std::trunc(loc_angle / (libMesh::pi / 3)) * (libMesh::pi / 3) - libMesh::pi / 3;
  Real x_coord_new = (std::cos(-rem_ang) * p(0) - std::sin(-rem_ang) * p(1));
  Real x_lim = (_n_rings - 1) * _pitch / 2.0;

  // looping over all channels
  for (unsigned int i = 0; i < _n_channels; i++)
  {
    // Distance from the point to subchannel
    distance1 = std::sqrt(std::pow((p(0) - _subchannel_position[i][0]), 2.0) +
                          std::pow((p(1) - _subchannel_position[i][1]), 2.0));

    // If subchannel belongs to center ring
    if (channel_distance < distance_outer_ring)
    {
      if ((distance1 < distance0) && (_subch_type[i] == EChannelType::CENTER))
      {
        j = i;
        distance0 = distance1;
      } // if
    } // if
    // If subchannel belongs to outer ring
    else
    {
      if ((distance1 < distance0) &&
          (_subch_type[i] == EChannelType::EDGE || _subch_type[i] == EChannelType::CORNER))
      {
        if (((x_coord_new > x_lim) || (x_coord_new < -x_lim)) &&
            _subch_type[i] == EChannelType::CORNER)
        {
          j = i;
          distance0 = distance1;
        } // if
        else if (((x_coord_new > x_lim) || (x_coord_new > -x_lim)) &&
                 _subch_type[i] == EChannelType::EDGE)
        {
          j = i;
          distance0 = distance1;
        } // if
      }   // if
    }     // else

  }   // for
  return j;
}

void
TriSubChannelMesh::buildMesh()
{
}

unsigned int
TriSubChannelMesh::getPinIndexFromPoint(const Point & p) const
{
  /// Function that returns the pin number given a point

  return this->pinIndex(p);
}

unsigned int
TriSubChannelMesh::pinIndex(const Point & p) const
{
  /// Function that returns the pin number given a point

  // Define the current ring
  Real distance_rod;
  Real d0 = 1e5;
  unsigned int current_rod = 0;

  std::vector<Point> positions;
  Point center(0, 0);
  this->rodPositions(positions, _n_rings, _pitch, center);
  for (unsigned int i = 0; i < _nrods; i++)
  {
    Real x_dist = positions[i](0) - p(0);
    Real y_dist = positions[i](1) - p(1);
    distance_rod = std::sqrt(std::pow(x_dist, 2) + std::pow(y_dist, 2));
    if (distance_rod < d0)
    {
      d0 = distance_rod;
      current_rod = i;
    }
  }

  return current_rod;
}

void
TriSubChannelMesh::rodPositions(std::vector<Point> & positions,
                                unsigned int nrings,
                                Real pitch,
                                Point center)
{
  /// Defining parameters
  // distance: it is the distance to the next rod
  //
  Real theta = 0.0;
  Real dtheta = 0.0;
  Real distance = 0.0;
  Real theta1 = 0.0;
  Real theta_corrected = 0.0;
  Real pi = libMesh::pi;
  unsigned int k = 0;
  positions.emplace_back(0.0, 0.0);
  for (unsigned int i = 1; i < nrings; i++)
  {
    dtheta = 2.0 * pi / (i * 6);
    theta = 0.0;
    for (unsigned int j = 0; j < i * 6; j++)
    {
      k = k + 1;
      theta1 = fmod(theta + 1.0e-10, pi / 3.0);
      distance = std::sqrt((pow(i * pitch, 2) + pow(theta1 / dtheta * pitch, 2) -
                            2.0 * i * pitch * (theta1 / dtheta * pitch) * std::cos(pi / 3.0)));
      theta_corrected = std::acos(
          1.0 / (i * pitch) / distance / 2.0 *
          (std::pow(i * pitch, 2) + std::pow(distance, 2) - std::pow(theta1 / dtheta * pitch, 2)));
      if (theta1 < 1.0e-6)
      {
        theta_corrected = theta;
      }
      else
      {
        if (theta > pi / 3.0 && theta <= 2.0 / 3.0 * pi)
          theta_corrected = theta_corrected + pi / 3.0;
        else if (theta > 2.0 / 3.0 * pi && theta <= pi)
          theta_corrected = theta_corrected + 2.0 / 3.0 * pi;
        else if (theta > pi && theta <= 4.0 / 3.0 * pi)
          theta_corrected = theta_corrected + pi;
        else if (theta > 4.0 / 3.0 * pi && theta <= 5.0 / 3.0 * pi)
          theta_corrected = theta_corrected + 4.0 / 3.0 * pi;
        else if (theta > 5.0 / 3.0 * pi && theta <= 2.0 * pi)
          theta_corrected = theta_corrected + 5.0 / 3.0 * pi;
      }
      positions.emplace_back(center(0) + distance * std::cos(theta_corrected),
                             center(1) + distance * std::sin(theta_corrected));
      theta = theta + dtheta;
    } // j
  }   // i
}

void
TriSubChannelMesh::setChannelToDuctMaps(const std::vector<Node *> & duct_nodes)
{
  const Real tol = 1e-10;
  for (size_t i = 0; i < duct_nodes.size(); i++)
  {
    int min_chan = 0;
    Real min_dist = std::numeric_limits<double>::max();
    Point ductpos((*duct_nodes[i])(0), (*duct_nodes[i])(1), 0);
    for (size_t j = 0; j < _subchannel_position.size(); j++)
    {
      Point chanpos(_subchannel_position[j][0], _subchannel_position[j][1], 0);
      auto dist = (chanpos - ductpos).norm();
      if (dist < min_dist)
      {
        min_dist = dist;
        min_chan = j;
      }
    }

    Node * chan_node = nullptr;
    for (auto cn : _nodes[min_chan])
    {
      if (std::abs((*cn)(2) - (*duct_nodes[i])(2)) < tol)
      {
        chan_node = cn;
        break;
      }
    }

    if (chan_node == nullptr)
      mooseError("failed to find matching channel node for duct node");

    _duct_node_to_chan_map[duct_nodes[i]] = chan_node;
    _chan_to_duct_node_map[chan_node] = duct_nodes[i];
  }

  _duct_nodes = duct_nodes;
}
