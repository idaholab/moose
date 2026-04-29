//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriSubChannelMesh.h"
#include <cmath>
#include "libmesh/node.h"

registerMooseObject("SubChannelApp", TriSubChannelMesh);

InputParameters
TriSubChannelMesh::validParams()
{
  InputParameters params = SubChannelMesh::validParams();
  params.addClassDescription("Creates an subchannel mesh container for a triangular "
                             "lattice arrangement");
  return params;
}

TriSubChannelMesh::TriSubChannelMesh(const InputParameters & params) : SubChannelMesh(params) {}

TriSubChannelMesh::TriSubChannelMesh(const TriSubChannelMesh & other_mesh)
  : SubChannelMesh(other_mesh),
    _n_rings(other_mesh._n_rings),
    _n_channels(other_mesh._n_channels),
    _flat_to_flat(other_mesh._flat_to_flat),
    _dwire(other_mesh._dwire),
    _hwire(other_mesh._hwire),
    _duct_to_pin_gap(other_mesh._duct_to_pin_gap),
    _nodes(other_mesh._nodes),
    _pin_nodes(other_mesh._pin_nodes),
    _gap_to_chan_map(other_mesh._gap_to_chan_map),
    _gap_to_pin_map(other_mesh._gap_to_pin_map),
    _chan_to_gap_map(other_mesh._chan_to_gap_map),
    _sign_id_crossflow_map(other_mesh._sign_id_crossflow_map),
    _gij_map(other_mesh._gij_map),
    _pin_position(other_mesh._pin_position),
    _pins_in_rings(other_mesh._pins_in_rings),
    _chan_to_pin_map(other_mesh._chan_to_pin_map),
    _npins(other_mesh._npins),
    _n_gaps(other_mesh._n_gaps),
    _subch_type(other_mesh._subch_type),
    _gap_type(other_mesh._gap_type),
    _gap_pairs_sf(other_mesh._gap_pairs_sf),
    _chan_pairs_sf(other_mesh._chan_pairs_sf),
    _pin_to_chan_map(other_mesh._pin_to_chan_map)
{
}

std::unique_ptr<MooseMesh>
TriSubChannelMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
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
  Real distance_outer_ring = _flat_to_flat / 2 - _duct_to_pin_gap - _pin_diameter / 2;
  Real channel_distance = std::sqrt(std::pow(p(0), 2) + std::pow(p(1), 2));
  Real angle = std::abs(std::atan2(p(1), p(0)));
  Real projection_angle =
      angle - libMesh::pi / 6 - std::trunc(angle / (libMesh::pi / 3)) * (libMesh::pi / 3);
  channel_distance = channel_distance * std::cos(projection_angle);

  // Projecting point on top edge to determine if the point is a corner or edge subchannel by x
  // coordinate
  Real loc_angle = std::atan2(p(1), p(0));
  if (loc_angle < 0.0)
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
        else if ((x_coord_new <= x_lim && x_coord_new >= -x_lim) &&
                 _subch_type[i] == EChannelType::EDGE)
        {
          j = i;
          distance0 = distance1;
        }
      }
    }
  }
  return j;
}

void
TriSubChannelMesh::buildMesh()
{
}

void
TriSubChannelMesh::computeAssemblyHydraulicParameters()
{
  _assembly_flow_area = 0.0;
  _assembly_wetted_perimeter = 0.0;
  _assembly_hydraulic_diameter = 0.0;

  const Real z = _z_grid.empty() ? 0.0 : _z_grid.front();

  for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
  {
    _assembly_flow_area += getSubchannelFlowArea(i_ch, z);
    _assembly_wetted_perimeter += getSubchannelWettedPerimeter(i_ch);
  }

  if (_assembly_wetted_perimeter == 0.0)
    mooseError(name(), ": Assembly wetted perimeter is zero; cannot compute hydraulic diameter.");

  _assembly_hydraulic_diameter = 4.0 * _assembly_flow_area / _assembly_wetted_perimeter;
}

Real
TriSubChannelMesh::getSubchannelFlowArea(unsigned int i_chan, Real z) const
{
  Real standard_area = 0.0;
  Real rod_area = 0.0;
  Real wire_area = 0.0;

  const Real theta =
      std::acos(_hwire / std::sqrt(std::pow(_hwire, 2) +
                                   std::pow(libMesh::pi * (_pin_diameter + _dwire), 2)));
  const auto subch_type = getSubchannelType(i_chan);
  if (subch_type == EChannelType::CENTER)
  {
    standard_area = std::pow(_pitch, 2) * std::sqrt(3.0) / 4.0;
    rod_area = libMesh::pi * std::pow(_pin_diameter, 2) / 8.0;
    wire_area = libMesh::pi * std::pow(_dwire, 2) / 8.0 / std::cos(theta);
  }
  else if (subch_type == EChannelType::EDGE)
  {
    standard_area = _pitch * (_pin_diameter / 2.0 + _duct_to_pin_gap);
    rod_area = libMesh::pi * std::pow(_pin_diameter, 2) / 8.0;
    wire_area = libMesh::pi * std::pow(_dwire, 2) / 8.0 / std::cos(theta);
  }
  else
  {
    standard_area = 1.0 / std::sqrt(3.0) * std::pow(_pin_diameter / 2.0 + _duct_to_pin_gap, 2);
    rod_area = libMesh::pi * std::pow(_pin_diameter, 2) / 24.0;
    wire_area = libMesh::pi * std::pow(_dwire, 2) / 24.0 / std::cos(theta);
  }

  Real flow_area = standard_area - rod_area - wire_area;

  unsigned int blockage_index = 0;
  for (const auto & i_blockage : _index_blockage)
  {
    if (i_chan == i_blockage && z >= _z_blockage.front() && z <= _z_blockage.back())
      flow_area *= _reduction_blockage[blockage_index];
    blockage_index++;
  }

  return flow_area;
}

Real
TriSubChannelMesh::getSubchannelWettedPerimeter(unsigned int i_chan) const
{
  const Real theta =
      std::acos(_hwire / std::sqrt(std::pow(_hwire, 2) +
                                   std::pow(libMesh::pi * (_pin_diameter + _dwire), 2)));
  const Real rod_circumference = libMesh::pi * _pin_diameter;
  const Real wire_circumference = libMesh::pi * _dwire;
  const auto subch_type = getSubchannelType(i_chan);

  if (subch_type == EChannelType::CENTER)
    return 0.5 * rod_circumference + 0.5 * wire_circumference / std::cos(theta);
  else if (subch_type == EChannelType::EDGE)
    return 0.5 * rod_circumference + 0.5 * wire_circumference / std::cos(theta) + _pitch;
  else
    return (rod_circumference + wire_circumference / std::cos(theta)) / 6.0 +
           2.0 / std::sqrt(3.0) * (_pin_diameter / 2.0 + _duct_to_pin_gap);
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
  this->pinPositions(positions, _n_rings, _pitch, center);
  for (unsigned int i = 0; i < _npins; i++)
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
TriSubChannelMesh::pinPositions(std::vector<Point> & positions,
                                unsigned int nrings,
                                Real pitch,
                                Point center)
{
  /// Defining parameters
  // distance: it is the distance to the next Pin
  //
  Real theta = 0.0;
  Real dtheta = 0.0;
  Real distance = 0.0;
  Real theta1 = 0.0;
  Real theta_corrected = 0.0;
  Real pi = libMesh::pi;
  unsigned int k = 0;
  positions.emplace_back(center(0), center(1));
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
      double argument = 1.0 / (i * pitch) / distance / 2.0 *
                        (std::pow(i * pitch, 2.0) + std::pow(distance, 2.0) -
                         std::pow(theta1 / dtheta * pitch, 2.0));
      // Check if the argument to std::acos() is within the valid range [-1, 1]
      if (argument >= -1.0 && argument <= 1.0)
      {
        theta_corrected = std::acos(argument);
      }
      else if (argument > 1.0)
      {
        theta_corrected = 0.0;
      }
      else
      {
        theta_corrected = pi;
      }
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
  } // i
}
