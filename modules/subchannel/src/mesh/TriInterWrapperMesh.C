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

#include "TriInterWrapperMesh.h"
#include <cmath>
#include "libmesh/node.h"

registerMooseObject("SubChannelApp", TriInterWrapperMesh);

InputParameters
TriInterWrapperMesh::validParams()
{
  InputParameters params = InterWrapperMesh::validParams();
  params.addClassDescription("Creates an inter-wrappper mesh container arround a triangular "
                             "lattice subchannel arrangement");
  return params;
}

TriInterWrapperMesh::TriInterWrapperMesh(const InputParameters & params) : InterWrapperMesh(params)
{
}

TriInterWrapperMesh::TriInterWrapperMesh(const TriInterWrapperMesh & other_mesh)
  : InterWrapperMesh(other_mesh),
    _n_rings(other_mesh._n_rings),
    _n_channels(other_mesh._n_channels),
    _flat_to_flat(other_mesh._flat_to_flat),
    _duct_to_rod_gap(other_mesh._duct_to_rod_gap),
    _nodes(other_mesh._nodes),
    _duct_nodes(other_mesh._duct_nodes),
    _chan_to_duct_node_map(other_mesh._chan_to_duct_node_map),
    _duct_node_to_chan_map(other_mesh._duct_node_to_chan_map),
    _gap_to_chan_map(other_mesh._gap_to_chan_map),
    _chan_to_gap_map(other_mesh._chan_to_gap_map),
    _sign_id_crossflow_map(other_mesh._sign_id_crossflow_map),
    _gij_map(other_mesh._gij_map),
    _subchannel_position(other_mesh._subchannel_position),
    _rod_position(other_mesh._rod_position),
    _rods_in_rings(other_mesh._rods_in_rings),
    _subchannel_to_rod_map(other_mesh._subchannel_to_rod_map),
    _gap_to_rod_map(other_mesh._gap_to_rod_map),
    _n_assemblies(other_mesh._n_assemblies),
    _n_gaps(other_mesh._n_gaps),
    _subch_type(other_mesh._subch_type),
    _gap_type(other_mesh._gap_type),
    _gap_pairs_sf(other_mesh._gap_pairs_sf),
    _chan_pairs_sf(other_mesh._chan_pairs_sf),
    _pin_to_chan_map(other_mesh._pin_to_chan_map),
    _tight_side_bypass(other_mesh._tight_side_bypass)
{
}

std::unique_ptr<MooseMesh>
TriInterWrapperMesh::safeClone() const
{
  return libmesh_make_unique<TriInterWrapperMesh>(*this);
}

unsigned int
TriInterWrapperMesh::getSubchannelIndexFromPoint(const Point & p) const
{
  Real distance0 = 1.0e+8;
  Real distance1;
  unsigned int j = 0;

  for (unsigned int i = 0; i < _n_channels; i++)
  {
    distance1 = std::sqrt(std::pow((p(0) - _subchannel_position[i][0]), 2.0) +
                          std::pow((p(1) - _subchannel_position[i][1]), 2.0));

    if (distance1 < distance0)
    {
      j = i;
      distance0 = distance1;
    } // if
  }   // for
  return j;
}

unsigned int
TriInterWrapperMesh::channelIndex(const Point & /*point*/) const
{
  // FIXME:
  return 0;
}

void
TriInterWrapperMesh::buildMesh()
{
}

unsigned int
TriInterWrapperMesh::getPinIndexFromPoint(const Point & /*p*/) const
{
  // TODO: implement routine that returns rod index given a point in 3D space
  return 0;
}

unsigned int
TriInterWrapperMesh::pinIndex(const Point & /*p*/) const
{
  // TODO: implement routine that returns rod index given a point in 3D space
  return 0;
}

void
TriInterWrapperMesh::rodPositions(std::vector<Point> & positions,
                                  unsigned int nrings,
                                  Real assembly_pitch,
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
      distance = std::sqrt(
          (pow(i * assembly_pitch, 2) + pow(theta1 / dtheta * assembly_pitch, 2) -
           2.0 * i * assembly_pitch * (theta1 / dtheta * assembly_pitch) * std::cos(pi / 3.0)));
      theta_corrected = std::acos(1.0 / (i * assembly_pitch) / distance / 2.0 *
                                  (std::pow(i * assembly_pitch, 2) + std::pow(distance, 2) -
                                   std::pow(theta1 / dtheta * assembly_pitch, 2)));
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
TriInterWrapperMesh::setChannelToDuctMaps(const std::vector<Node *> & duct_nodes)
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
