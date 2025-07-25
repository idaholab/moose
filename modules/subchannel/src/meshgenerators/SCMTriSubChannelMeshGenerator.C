//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTriSubChannelMeshGenerator.h"
#include "TriSubChannelMesh.h"
#include <cmath>
#include "libmesh/edge_edge2.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", SCMTriSubChannelMeshGenerator);
registerMooseObjectRenamed("SubChannelApp",
                           TriSubChannelMeshGenerator,
                           "06/30/2025 24:00",
                           SCMTriSubChannelMeshGenerator);

InputParameters
SCMTriSubChannelMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Creates a mesh of 1D subchannels in a triangular lattice arrangement");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addRequiredParam<Real>("pitch", "Pitch [m]");
  params.addRequiredParam<Real>("pin_diameter", "Rod diameter [m]");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addRequiredParam<unsigned int>("nrings", "Number of fuel Pin rings per assembly [-]");
  params.addRequiredParam<Real>("flat_to_flat",
                                "Flat to flat distance for the hexagonal assembly [m]");
  params.addRequiredParam<Real>("dwire", "Wire diameter [m]");
  params.addRequiredParam<Real>("hwire", "Wire lead length [m]");
  params.addParam<std::vector<Real>>(
      "spacer_z", {}, "Axial location of spacers/vanes/mixing_vanes [m]");
  params.addParam<std::vector<Real>>(
      "spacer_k", {}, "K-loss coefficient of spacers/vanes/mixing_vanes [-]");
  params.addParam<Real>("Kij", 0.5, "Lateral form loss coefficient [-]");
  params.addParam<std::vector<Real>>("z_blockage",
                                     std::vector<Real>({0.0, 0.0}),
                                     "axial location of blockage (inlet, outlet) [m]");
  params.addParam<std::vector<unsigned int>>("index_blockage",
                                             std::vector<unsigned int>({0}),
                                             "index of subchannels affected by blockage");
  params.addParam<std::vector<Real>>(
      "reduction_blockage",
      std::vector<Real>({1.0}),
      "Area reduction of subchannels affected by blockage (number to muliply the area)");
  params.addParam<std::vector<Real>>("k_blockage",
                                     std::vector<Real>({0.0}),
                                     "Form loss coefficient of subchannels affected by blockage");
  params.addParam<unsigned int>("block_id", 0, "Domain Index");
  return params;
}

SCMTriSubChannelMeshGenerator::SCMTriSubChannelMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _unheated_length_entry(getParam<Real>("unheated_length_entry")),
    _heated_length(getParam<Real>("heated_length")),
    _unheated_length_exit(getParam<Real>("unheated_length_exit")),
    _block_id(getParam<unsigned int>("block_id")),
    _spacer_z(getParam<std::vector<Real>>("spacer_z")),
    _spacer_k(getParam<std::vector<Real>>("spacer_k")),
    _z_blockage(getParam<std::vector<Real>>("z_blockage")),
    _index_blockage(getParam<std::vector<unsigned int>>("index_blockage")),
    _reduction_blockage(getParam<std::vector<Real>>("reduction_blockage")),
    _k_blockage(getParam<std::vector<Real>>("k_blockage")),
    _pitch(getParam<Real>("pitch")),
    _kij(getParam<Real>("Kij")),
    _pin_diameter(getParam<Real>("pin_diameter")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _n_rings(getParam<unsigned int>("nrings")),
    _flat_to_flat(getParam<Real>("flat_to_flat")),
    _dwire(getParam<Real>("dwire")),
    _hwire(getParam<Real>("hwire")),
    _duct_to_pin_gap(0.5 *
                     (_flat_to_flat - (_n_rings - 1) * _pitch * std::sqrt(3.0) - _pin_diameter))
{
  if (_spacer_z.size() != _spacer_k.size())
    mooseError(name(), ": Size of vector spacer_z should be equal to size of vector spacer_k");

  if (_spacer_z.size() &&
      _spacer_z.back() > _unheated_length_entry + _heated_length + _unheated_length_exit)
    mooseError(name(), ": Location of spacers should be less than the total bundle length");

  if (_z_blockage.size() != 2)
    mooseError(name(), ": Size of vector z_blockage must be 2");

  if (*max_element(_reduction_blockage.begin(), _reduction_blockage.end()) > 1)
    mooseError(name(), ": The area reduction of the blocked subchannels cannot be more than 1");

  if ((_index_blockage.size() != _reduction_blockage.size()) ||
      (_index_blockage.size() != _k_blockage.size()) ||
      (_reduction_blockage.size() != _k_blockage.size()))
    mooseError(name(),
               ": Size of vectors: index_blockage, reduction_blockage, k_blockage, must be equal "
               "to eachother");

  SubChannelMesh::generateZGrid(
      _unheated_length_entry, _heated_length, _unheated_length_exit, _n_cells, _z_grid);

  // Defining the total length from 3 axial sections
  Real L = _unheated_length_entry + _heated_length + _unheated_length_exit;

  // Defining the position of the spacer grid in the numerical solution array
  std::vector<int> spacer_cell;
  for (const auto & elem : _spacer_z)
    spacer_cell.emplace_back(std::round(elem * _n_cells / L));

  // Defining the array for axial resistances
  std::vector<Real> kgrid;
  kgrid.resize(_n_cells + 1, 0.0);

  // Summing the spacer resistance to the grid resistance array
  for (unsigned int index = 0; index < spacer_cell.size(); index++)
    kgrid[spacer_cell[index]] += _spacer_k[index];

  //  compute the hex mesh variables
  // -------------------------------------------

  // x coordinate for the first position
  Real x0 = 0.0;
  // y coordinate for the first position
  Real y0 = 0.0;
  // x coordinate for the second position
  Real x1 = 0.0;
  // y coordinate for the second position dummy variable
  Real y1 = 0.0;
  // dummy variable
  Real a1 = 0.0;
  // dummy variable
  Real a2 = 0.0;
  // average x coordinate
  Real avg_coor_x = 0.0;
  // average y coordinate
  Real avg_coor_y = 0.0;
  // distance between two points
  Real dist = 0.0;
  // distance between two points
  Real dist0 = 0.0;
  // integer counter
  unsigned int kgap = 0;
  // dummy integer
  unsigned int icorner = 0;
  // used to defined global direction of the cross_flow_map coefficients for each subchannel and gap
  const Real positive_flow = 1.0;
  // used to defined global direction of the cross_flow_map coefficients for each subchannel and gap
  const Real negative_flow = -1.0;
  // the indicator used while setting _gap_to_chan_map array
  std::vector<std::pair<unsigned int, unsigned int>> gap_fill;
  TriSubChannelMesh::rodPositions(_pin_position, _n_rings, _pitch, Point(0, 0));
  _npins = _pin_position.size();
  // assign the pins to the corresponding rings
  unsigned int k = 0; // initialize the fuel Pin counter index
  _pins_in_rings.resize(_n_rings);
  _pins_in_rings[0].push_back(k++);
  for (unsigned int i = 1; i < _n_rings; i++)
    for (unsigned int j = 0; j < i * 6; j++)
      _pins_in_rings[i].push_back(k++);
  //  Given the number of pins and number of fuel Pin rings, the number of subchannels can be
  //  computed as follows:
  unsigned int chancount = 0.0;
  // Summing internal channels
  for (unsigned int j = 0; j < _n_rings - 1; j++)
    chancount += j * 6;
  // Adding external channels to the total count
  _n_channels = chancount + _npins - 1 + (_n_rings - 1) * 6 + 6;

  if (*max_element(_index_blockage.begin(), _index_blockage.end()) > (_n_channels - 1))
    mooseError(name(),
               ": The index of the blocked subchannel cannot be more than the max index of the "
               "subchannels");

  if ((_index_blockage.size() > _n_channels) || (_reduction_blockage.size() > _n_channels) ||
      (_k_blockage.size() > _n_channels))
    mooseError(name(),
               ": Size of vectors: index_blockage, reduction_blockage, k_blockage, cannot be more "
               "than the total number of subchannels");

  // Defining the 2D array for axial resistances
  _k_grid.resize(_n_channels, std::vector<Real>(_n_cells + 1));
  for (unsigned int i = 0; i < _n_channels; i++)
    _k_grid[i] = kgrid;

  // Add blockage resistance to the 2D grid resistane array
  Real dz = L / _n_cells;
  for (unsigned int i = 0; i < _n_cells + 1; i++)
  {
    if ((dz * i >= _z_blockage.front() && dz * i <= _z_blockage.back()))
    {
      unsigned int index(0);
      for (const auto & i_ch : _index_blockage)
      {
        _k_grid[i_ch][i] += _k_blockage[index];
        index++;
      }
    }
  }

  _chan_to_pin_map.resize(_n_channels);
  _pin_to_chan_map.resize(_npins);
  _subch_type.resize(_n_channels);
  _n_gaps = _n_channels + _npins - 1; /// initial assignment
  _gap_to_chan_map.resize(_n_gaps);
  _gap_to_pin_map.resize(_n_gaps);
  gap_fill.resize(_n_gaps);
  _chan_to_gap_map.resize(_n_channels);
  _gap_pairs_sf.resize(_n_channels);
  _chan_pairs_sf.resize(_n_channels);
  _gij_map.resize(_n_cells + 1);
  _sign_id_crossflow_map.resize(_n_channels);
  _gap_type.resize(_n_gaps);
  _subchannel_position.resize(_n_channels);

  for (unsigned int i = 0; i < _n_channels; i++)
  {
    _chan_to_pin_map[i].reserve(3);
    _chan_to_gap_map[i].reserve(3);
    _sign_id_crossflow_map[i].reserve(3);
    _subchannel_position[i].reserve(3);
    for (unsigned int j = 0; j < 3; j++)
    {
      _sign_id_crossflow_map.at(i).push_back(positive_flow);
      _subchannel_position.at(i).push_back(0.0);
    }
  }

  for (unsigned int iz = 0; iz < _n_cells + 1; iz++)
  {
    _gij_map[iz].reserve(_n_gaps);
  }

  for (unsigned int i = 0; i < _npins; i++)
    _pin_to_chan_map[i].reserve(6);

  // create the subchannels
  k = 0; // initialize the subchannel counter index
  kgap = 0;
  // for each ring we trace the subchannels by pairing up to neighbor pins and looking for the third
  // Pin at inner or outer ring compared to the current ring.
  for (unsigned int i = 1; i < _n_rings; i++)
  {
    // find the closest Pin at back ring
    for (unsigned int j = 0; j < _pins_in_rings[i].size(); j++)
    {
      if (j == _pins_in_rings[i].size() - 1)
      {
        _chan_to_pin_map[k].push_back(_pins_in_rings[i][j]);
        _chan_to_pin_map[k].push_back(_pins_in_rings[i][0]);
        avg_coor_x =
            0.5 * (_pin_position[_pins_in_rings[i][j]](0) + _pin_position[_pins_in_rings[i][0]](0));
        avg_coor_y =
            0.5 * (_pin_position[_pins_in_rings[i][j]](1) + _pin_position[_pins_in_rings[i][0]](1));
        _gap_to_pin_map[kgap].first = _pins_in_rings[i][0];
        _gap_to_pin_map[kgap].second = _pins_in_rings[i][j];
        _gap_type[kgap] = EChannelType::CENTER;
        kgap = kgap + 1;
      }
      else
      {
        _chan_to_pin_map[k].push_back(_pins_in_rings[i][j]);
        _chan_to_pin_map[k].push_back(_pins_in_rings[i][j + 1]);
        avg_coor_x = 0.5 * (_pin_position[_pins_in_rings[i][j]](0) +
                            _pin_position[_pins_in_rings[i][j + 1]](0));
        avg_coor_y = 0.5 * (_pin_position[_pins_in_rings[i][j]](1) +
                            _pin_position[_pins_in_rings[i][j + 1]](1));
        _gap_to_pin_map[kgap].first = _pins_in_rings[i][j];
        _gap_to_pin_map[kgap].second = _pins_in_rings[i][j + 1];
        _gap_type[kgap] = EChannelType::CENTER;
        kgap = kgap + 1;
      }

      dist0 = 1.0e+5;

      _chan_to_pin_map[k].push_back(_pins_in_rings[i - 1][0]);
      unsigned int l0 = 0;

      for (unsigned int l = 0; l < _pins_in_rings[i - 1].size(); l++)
      {
        dist = std::sqrt(pow(_pin_position[_pins_in_rings[i - 1][l]](0) - avg_coor_x, 2) +
                         pow(_pin_position[_pins_in_rings[i - 1][l]](1) - avg_coor_y, 2));

        if (dist < dist0)
        {
          _chan_to_pin_map[k][2] = _pins_in_rings[i - 1][l];
          l0 = l;
          dist0 = dist;
        } // if
      } // l

      _gap_to_pin_map[kgap].first = _pins_in_rings[i][j];
      _gap_to_pin_map[kgap].second = _pins_in_rings[i - 1][l0];
      _gap_type[kgap] = EChannelType::CENTER;
      kgap = kgap + 1;
      _subch_type[k] = EChannelType::CENTER;
      k = k + 1;

    } // for j

    // find the closest Pin at front ring

    for (unsigned int j = 0; j < _pins_in_rings[i].size(); j++)
    {
      if (j == _pins_in_rings[i].size() - 1)
      {
        _chan_to_pin_map[k].push_back(_pins_in_rings[i][j]);
        _chan_to_pin_map[k].push_back(_pins_in_rings[i][0]);
        avg_coor_x =
            0.5 * (_pin_position[_pins_in_rings[i][j]](0) + _pin_position[_pins_in_rings[i][0]](0));
        avg_coor_y =
            0.5 * (_pin_position[_pins_in_rings[i][j]](1) + _pin_position[_pins_in_rings[i][0]](1));
      }
      else
      {
        _chan_to_pin_map[k].push_back(_pins_in_rings[i][j]);
        _chan_to_pin_map[k].push_back(_pins_in_rings[i][j + 1]);
        avg_coor_x = 0.5 * (_pin_position[_pins_in_rings[i][j]](0) +
                            _pin_position[_pins_in_rings[i][j + 1]](0));
        avg_coor_y = 0.5 * (_pin_position[_pins_in_rings[i][j]](1) +
                            _pin_position[_pins_in_rings[i][j + 1]](1));
      }

      // if the outermost ring, set the edge subchannels first... then the corner subchannels
      if (i == _n_rings - 1)
      {
        // add  edges
        _subch_type[k] = EChannelType::EDGE; // an edge subchannel is created
        _gap_to_pin_map[kgap].first = _pins_in_rings[i][j];
        _gap_to_pin_map[kgap].second = _pins_in_rings[i][j];
        _gap_type[kgap] = EChannelType::EDGE;
        _chan_to_gap_map[k].push_back(kgap);
        kgap = kgap + 1;
        k = k + 1;

        if (j % i == 0)
        {
          // generate a corner subchannel, generate the additional gap and fix chan_to_gap_map
          _gap_to_pin_map[kgap].first = _pins_in_rings[i][j];
          _gap_to_pin_map[kgap].second = _pins_in_rings[i][j];
          _gap_type[kgap] = EChannelType::CORNER;

          // corner subchannel
          _chan_to_pin_map[k].push_back(_pins_in_rings[i][j]);
          _chan_to_gap_map[k].push_back(kgap - 1);
          _chan_to_gap_map[k].push_back(kgap);
          _subch_type[k] = EChannelType::CORNER;

          kgap = kgap + 1;
          k = k + 1;
        }
        // if not the outer most ring
      }
      else
      {
        dist0 = 1.0e+5;
        unsigned int l0 = 0;
        _chan_to_pin_map[k].push_back(_pins_in_rings[i + 1][0]);
        for (unsigned int l = 0; l < _pins_in_rings[i + 1].size(); l++)
        {
          dist = std::sqrt(pow(_pin_position[_pins_in_rings[i + 1][l]](0) - avg_coor_x, 2) +
                           pow(_pin_position[_pins_in_rings[i + 1][l]](1) - avg_coor_y, 2));
          if (dist < dist0)
          {
            _chan_to_pin_map[k][2] = _pins_in_rings[i + 1][l];
            dist0 = dist;
            l0 = l;
          } // if
        } // l

        _gap_to_pin_map[kgap].first = _pins_in_rings[i][j];
        _gap_to_pin_map[kgap].second = _pins_in_rings[i + 1][l0];
        _gap_type[kgap] = EChannelType::CENTER;
        kgap = kgap + 1;
        _subch_type[k] = EChannelType::CENTER;
        k = k + 1;
      } // if
    } // for j
  } // for i

  // Constructing pins to channels mao
  for (unsigned int loc_rod = 0; loc_rod < _npins; loc_rod++)
  {
    for (unsigned int i = 0; i < _n_channels; i++)
    {
      bool rod_in_sc = false;
      for (unsigned int j : _chan_to_pin_map[i])
      {
        if (j == loc_rod)
          rod_in_sc = true;
      }
      if (rod_in_sc)
      {
        _pin_to_chan_map[loc_rod].push_back(i);
      }
    }
  }

  // find the _gap_to_chan_map and _chan_to_gap_map using the gap_to_rod and subchannel_to_rod_maps

  for (unsigned int i = 0; i < _n_channels; i++)
  {
    if (_subch_type[i] == EChannelType::CENTER)
    {
      for (unsigned int j = 0; j < _n_gaps; j++)
      {
        if (_gap_type[j] == EChannelType::CENTER)
        {
          if (((_chan_to_pin_map[i][0] == _gap_to_pin_map[j].first) &&
               (_chan_to_pin_map[i][1] == _gap_to_pin_map[j].second)) ||
              ((_chan_to_pin_map[i][0] == _gap_to_pin_map[j].second) &&
               (_chan_to_pin_map[i][1] == _gap_to_pin_map[j].first)))
          {
            _chan_to_gap_map[i].push_back(j);
          }

          if (((_chan_to_pin_map[i][0] == _gap_to_pin_map[j].first) &&
               (_chan_to_pin_map[i][2] == _gap_to_pin_map[j].second)) ||
              ((_chan_to_pin_map[i][0] == _gap_to_pin_map[j].second) &&
               (_chan_to_pin_map[i][2] == _gap_to_pin_map[j].first)))
          {
            _chan_to_gap_map[i].push_back(j);
          }

          if (((_chan_to_pin_map[i][1] == _gap_to_pin_map[j].first) &&
               (_chan_to_pin_map[i][2] == _gap_to_pin_map[j].second)) ||
              ((_chan_to_pin_map[i][1] == _gap_to_pin_map[j].second) &&
               (_chan_to_pin_map[i][2] == _gap_to_pin_map[j].first)))
          {
            _chan_to_gap_map[i].push_back(j);
          }
        }
      } // for j
    }
    else if (_subch_type[i] == EChannelType::EDGE)
    {
      for (unsigned int j = 0; j < _n_gaps; j++)
      {
        if (_gap_type[j] == EChannelType::CENTER)
        {
          if (((_chan_to_pin_map[i][0] == _gap_to_pin_map[j].first) &&
               (_chan_to_pin_map[i][1] == _gap_to_pin_map[j].second)) ||
              ((_chan_to_pin_map[i][0] == _gap_to_pin_map[j].second) &&
               (_chan_to_pin_map[i][1] == _gap_to_pin_map[j].first)))
          {
            _chan_to_gap_map[i].push_back(j);
          }
        }
      }

      icorner = 0;
      for (unsigned int k = 0; k < _n_channels; k++)
      {
        if (_subch_type[k] == EChannelType::CORNER &&
            _chan_to_pin_map[i][1] == _chan_to_pin_map[k][0])
        {
          _chan_to_gap_map[i].push_back(_chan_to_gap_map[k][1]);
          icorner = 1;
          break;
        } // if
      } // for

      for (unsigned int k = 0; k < _n_channels; k++)
      {
        if (_subch_type[k] == EChannelType::CORNER &&
            _chan_to_pin_map[i][0] == _chan_to_pin_map[k][0])
        {
          _chan_to_gap_map[i].push_back(_chan_to_gap_map[k][1] + 1);
          icorner = 1;
          break;
        }
      }

      if (icorner == 0)
      {
        _chan_to_gap_map[i].push_back(_chan_to_gap_map[i][0] + 1);
      }
    }
  }

  // find gap_to_chan_map pair

  for (unsigned int j = 0; j < _n_gaps; j++)
  {
    for (unsigned int i = 0; i < _n_channels; i++)
    {
      if (_subch_type[i] == EChannelType::CENTER || _subch_type[i] == EChannelType::EDGE)
      {
        if ((j == _chan_to_gap_map[i][0]) || (j == _chan_to_gap_map[i][1]) ||
            (j == _chan_to_gap_map[i][2]))
        {
          if (_gap_to_chan_map[j].first == 0 && gap_fill[j].first == 0)
          {
            _gap_to_chan_map[j].first = i;
            gap_fill[j].first = 1;
          }
          else if (_gap_to_chan_map[j].second == 0 && gap_fill[j].second == 0)
          {
            _gap_to_chan_map[j].second = i;
            gap_fill[j].second = 1;
          }
          else
          {
          }
        }
      }
      else if (_subch_type[i] == EChannelType::CORNER)
      {
        if ((j == _chan_to_gap_map[i][0]) || (j == _chan_to_gap_map[i][1]))
        {
          if (_gap_to_chan_map[j].first == 0 && gap_fill[j].first == 0)
          {
            _gap_to_chan_map[j].first = i;
            gap_fill[j].first = 1;
          }
          else if (_gap_to_chan_map[j].second == 0 && gap_fill[j].second == 0)
          {
            _gap_to_chan_map[j].second = i;
            gap_fill[j].second = 1;
          }
          else
          {
          }
        }
      }
    } // i
  } // j

  for (unsigned int k = 0; k < _n_channels; k++)
  {
    if (_subch_type[k] == EChannelType::EDGE)
    {
      _gap_pairs_sf[k].first = _chan_to_gap_map[k][0];
      _gap_pairs_sf[k].second = _chan_to_gap_map[k][2];
      auto k1 = _gap_pairs_sf[k].first;
      auto k2 = _gap_pairs_sf[k].second;
      if (_gap_to_chan_map[k1].first == k)
      {
        _chan_pairs_sf[k].first = _gap_to_chan_map[k1].second;
      }
      else
      {
        _chan_pairs_sf[k].first = _gap_to_chan_map[k1].first;
      }

      if (_gap_to_chan_map[k2].first == k)
      {
        _chan_pairs_sf[k].second = _gap_to_chan_map[k2].second;
      }
      else
      {
        _chan_pairs_sf[k].second = _gap_to_chan_map[k2].first;
      }
    }
    else if (_subch_type[k] == EChannelType::CORNER)
    {
      _gap_pairs_sf[k].first = _chan_to_gap_map[k][1];
      _gap_pairs_sf[k].second = _chan_to_gap_map[k][0];

      auto k1 = _gap_pairs_sf[k].first;
      auto k2 = _gap_pairs_sf[k].second;

      if (_gap_to_chan_map[k1].first == k)
      {
        _chan_pairs_sf[k].first = _gap_to_chan_map[k1].second;
      }
      else
      {
        _chan_pairs_sf[k].first = _gap_to_chan_map[k1].first;
      }

      if (_gap_to_chan_map[k2].first == k)
      {
        _chan_pairs_sf[k].second = _gap_to_chan_map[k2].second;
      }
      else
      {
        _chan_pairs_sf[k].second = _gap_to_chan_map[k2].first;
      }
    }
  }

  // set the _gij_map
  for (unsigned int iz = 0; iz < _n_cells + 1; iz++)
  {
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      if (_gap_type[i_gap] == EChannelType::CENTER)
      {
        _gij_map[iz].push_back(_pitch - _pin_diameter);
      }
      else if (_gap_type[i_gap] == EChannelType::EDGE || _gap_type[i_gap] == EChannelType::CORNER)
      {
        _gij_map[iz].push_back(_duct_to_pin_gap);
      }
    }
  }

  for (unsigned int i = 0; i < _n_channels; i++)
  {
    if (_subch_type[i] == EChannelType::CENTER || _subch_type[i] == EChannelType::EDGE)
    {
      for (unsigned int k = 0; k < 3; k++)
      {
        for (unsigned int j = 0; j < _n_gaps; j++)
        {
          if (_chan_to_gap_map[i][k] == j && i == _gap_to_chan_map[j].first)
          {
            if (i > _gap_to_chan_map[j].second)
            {
              _sign_id_crossflow_map[i][k] = negative_flow;
            }
            else
            {
              _sign_id_crossflow_map[i][k] = positive_flow;
            }
          }
          else if (_chan_to_gap_map[i][k] == j && i == _gap_to_chan_map[j].second)
          {
            if (i > _gap_to_chan_map[j].first)
            {
              _sign_id_crossflow_map[i][k] = negative_flow;
            }
            else
            {
              _sign_id_crossflow_map[i][k] = positive_flow;
            }
          }
        } // j
      } // k
    }
    else if (_subch_type[i] == EChannelType::CORNER)
    {
      for (unsigned int k = 0; k < 2; k++)
      {
        for (unsigned int j = 0; j < _n_gaps; j++)
        {
          if (_chan_to_gap_map[i][k] == j && i == _gap_to_chan_map[j].first)
          {
            if (i > _gap_to_chan_map[j].second)
            {
              _sign_id_crossflow_map[i][k] = negative_flow;
            }
            else
            {
              _sign_id_crossflow_map[i][k] = positive_flow;
            }
          }
          else if (_chan_to_gap_map[i][k] == j && i == _gap_to_chan_map[j].second)
          {
            if (i > _gap_to_chan_map[j].first)
            {
              _sign_id_crossflow_map[i][k] = negative_flow;
            }
            else
            {
              _sign_id_crossflow_map[i][k] = positive_flow;
            }
          }
        } // j
      } // k
    } // subch_type =2
  } // i

  // set the subchannel positions
  for (unsigned int i = 0; i < _n_channels; i++)
  {
    if (_subch_type[i] == EChannelType::CENTER)
    {
      _subchannel_position[i][0] =
          (_pin_position[_chan_to_pin_map[i][0]](0) + _pin_position[_chan_to_pin_map[i][1]](0) +
           _pin_position[_chan_to_pin_map[i][2]](0)) /
          3.0;
      _subchannel_position[i][1] =
          (_pin_position[_chan_to_pin_map[i][0]](1) + _pin_position[_chan_to_pin_map[i][1]](1) +
           _pin_position[_chan_to_pin_map[i][2]](1)) /
          3.0;
    }
    else if (_subch_type[i] == EChannelType::EDGE)
    {
      for (unsigned int j = 0; j < _n_channels; j++)
      {
        if (_subch_type[j] == EChannelType::CENTER &&
            ((_chan_to_pin_map[i][0] == _chan_to_pin_map[j][0] &&
              _chan_to_pin_map[i][1] == _chan_to_pin_map[j][1]) ||
             (_chan_to_pin_map[i][0] == _chan_to_pin_map[j][1] &&
              _chan_to_pin_map[i][1] == _chan_to_pin_map[j][0])))
        {
          x0 = _pin_position[_chan_to_pin_map[j][2]](0);
          y0 = _pin_position[_chan_to_pin_map[j][2]](1);
        }
        else if (_subch_type[j] == EChannelType::CENTER &&
                 ((_chan_to_pin_map[i][0] == _chan_to_pin_map[j][0] &&
                   _chan_to_pin_map[i][1] == _chan_to_pin_map[j][2]) ||
                  (_chan_to_pin_map[i][0] == _chan_to_pin_map[j][2] &&
                   _chan_to_pin_map[i][1] == _chan_to_pin_map[j][0])))
        {
          x0 = _pin_position[_chan_to_pin_map[j][1]](0);
          y0 = _pin_position[_chan_to_pin_map[j][1]](1);
        }
        else if (_subch_type[j] == EChannelType::CENTER &&
                 ((_chan_to_pin_map[i][0] == _chan_to_pin_map[j][1] &&
                   _chan_to_pin_map[i][1] == _chan_to_pin_map[j][2]) ||
                  (_chan_to_pin_map[i][0] == _chan_to_pin_map[j][2] &&
                   _chan_to_pin_map[i][1] == _chan_to_pin_map[j][1])))
        {
          x0 = _pin_position[_chan_to_pin_map[j][0]](0);
          y0 = _pin_position[_chan_to_pin_map[j][0]](1);
        }
        x1 = 0.5 *
             (_pin_position[_chan_to_pin_map[i][0]](0) + _pin_position[_chan_to_pin_map[i][1]](0));
        y1 = 0.5 *
             (_pin_position[_chan_to_pin_map[i][0]](1) + _pin_position[_chan_to_pin_map[i][1]](1));
        a1 = _pin_diameter / 2.0 + _duct_to_pin_gap / 2.0;
        a2 = std::sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) + a1;
        _subchannel_position[i][0] = (a2 * x1 - a1 * x0) / (a2 - a1);
        _subchannel_position[i][1] = (a2 * y1 - a1 * y0) / (a2 - a1);
      } // j
    }
    else if (_subch_type[i] == EChannelType::CORNER)
    {
      x0 = _pin_position[0](0);
      y0 = _pin_position[0](1);
      x1 = _pin_position[_chan_to_pin_map[i][0]](0);
      y1 = _pin_position[_chan_to_pin_map[i][0]](1);
      a1 = _pin_diameter / 2.0 + _duct_to_pin_gap / 2.0;
      a2 = std::sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) + a1;
      _subchannel_position[i][0] = (a2 * x1 - a1 * x0) / (a2 - a1);
      _subchannel_position[i][1] = (a2 * y1 - a1 * y0) / (a2 - a1);
    }
  }

  /// Special case _n_rings == 1
  if (_n_rings == 1)
  {
    for (unsigned int i = 0; i < _n_channels; i++)
    {
      Real angle = (2 * i + 1) * libMesh::pi / 6.0;
      _subch_type[i] = EChannelType::CORNER;
      _subchannel_position[i][0] = std::cos(angle) * _flat_to_flat / 2.0;
      _subchannel_position[i][1] = std::sin(angle) * _flat_to_flat / 2.0;
    }
  }

  // Reduce reserved memory in the channel-to-gap map.
  for (auto & gap : _chan_to_gap_map)
  {
    gap.shrink_to_fit();
  }
}

std::unique_ptr<MeshBase>
SCMTriSubChannelMeshGenerator::generate()
{
  auto mesh_base = buildMeshBaseObject();

  BoundaryInfo & boundary_info = mesh_base->get_boundary_info();
  mesh_base->set_spatial_dimension(3);
  mesh_base->reserve_elem(_n_cells * _n_channels);
  mesh_base->reserve_nodes((_n_cells + 1) * _n_channels);
  _nodes.resize(_n_channels);
  // Add the points for the give x,y subchannel positions.  The grid is hexagonal.
  //  The grid along
  // z is irregular to account for Pin spacers.  Store pointers in the _nodes
  // array so we can keep track of which points are in which channels.
  unsigned int node_id = 0;
  for (unsigned int i = 0; i < _n_channels; i++)
  {
    _nodes[i].reserve(_n_cells + 1);
    for (unsigned int iz = 0; iz < _n_cells + 1; iz++)
    {
      _nodes[i].push_back(mesh_base->add_point(
          Point(_subchannel_position[i][0], _subchannel_position[i][1], _z_grid[iz]), node_id++));
    }
  }

  // Add the elements which in this case are 2-node edges that link each
  // subchannel's nodes vertically.
  unsigned int elem_id = 0;
  for (unsigned int i = 0; i < _n_channels; i++)
  {
    for (unsigned int iz = 0; iz < _n_cells; iz++)
    {
      Elem * elem = new Edge2;
      elem->set_id(elem_id++);
      elem = mesh_base->add_elem(elem);
      const int indx1 = (_n_cells + 1) * i + iz;
      const int indx2 = (_n_cells + 1) * i + (iz + 1);
      elem->set_node(0, mesh_base->node_ptr(indx1));
      elem->set_node(1, mesh_base->node_ptr(indx2));

      if (iz == 0)
        boundary_info.add_side(elem, 0, 0);
      if (iz == _n_cells - 1)
        boundary_info.add_side(elem, 1, 1);
    }
  }
  boundary_info.sideset_name(0) = "inlet";
  boundary_info.sideset_name(1) = "outlet";
  boundary_info.nodeset_name(0) = "inlet";
  boundary_info.nodeset_name(1) = "outlet";

  // Naming the block
  mesh_base->subdomain_name(_block_id) = name();

  mesh_base->prepare_for_use();

  // move the meta data into TriSubChannelMesh
  auto & sch_mesh = static_cast<TriSubChannelMesh &>(*_mesh);
  sch_mesh._unheated_length_entry = _unheated_length_entry;
  sch_mesh._heated_length = _heated_length;
  sch_mesh._unheated_length_exit = _unheated_length_exit;
  sch_mesh._z_grid = _z_grid;
  sch_mesh._k_grid = _k_grid;
  sch_mesh._spacer_z = _spacer_z;
  sch_mesh._spacer_k = _spacer_k;
  sch_mesh._z_blockage = _z_blockage;
  sch_mesh._index_blockage = _index_blockage;
  sch_mesh._reduction_blockage = _reduction_blockage;
  sch_mesh._kij = _kij;
  sch_mesh._pitch = _pitch;
  sch_mesh._pin_diameter = _pin_diameter;
  sch_mesh._n_cells = _n_cells;
  sch_mesh._n_rings = _n_rings;
  sch_mesh._n_channels = _n_channels;
  sch_mesh._flat_to_flat = _flat_to_flat;
  sch_mesh._dwire = _dwire;
  sch_mesh._hwire = _hwire;
  sch_mesh._duct_to_pin_gap = _duct_to_pin_gap;
  sch_mesh._nodes = _nodes;
  sch_mesh._gap_to_chan_map = _gap_to_chan_map;
  sch_mesh._gap_to_pin_map = _gap_to_pin_map;
  sch_mesh._chan_to_gap_map = _chan_to_gap_map;
  sch_mesh._sign_id_crossflow_map = _sign_id_crossflow_map;
  sch_mesh._gij_map = _gij_map;
  sch_mesh._subchannel_position = _subchannel_position;
  sch_mesh._pin_position = _pin_position;
  sch_mesh._pins_in_rings = _pins_in_rings;
  sch_mesh._chan_to_pin_map = _chan_to_pin_map;
  sch_mesh._npins = _npins;
  sch_mesh._n_gaps = _n_gaps;
  sch_mesh._subch_type = _subch_type;
  sch_mesh._gap_type = _gap_type;
  sch_mesh._gap_pairs_sf = _gap_pairs_sf;
  sch_mesh._chan_pairs_sf = _chan_pairs_sf;
  sch_mesh._pin_to_chan_map = _pin_to_chan_map;

  return mesh_base;
}
