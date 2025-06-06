//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTriInterWrapperMeshGenerator.h"
#include "TriInterWrapperMesh.h"
#include <cmath>
#include "libmesh/edge_edge2.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", SCMTriInterWrapperMeshGenerator);
registerMooseObjectRenamed("SubChannelApp",
                           TriInterWrapperMeshGenerator,
                           "06/30/2025 24:00",
                           SCMTriInterWrapperMeshGenerator);

InputParameters
SCMTriInterWrapperMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Creates a mesh for the inter-wrapper around triangular subassemblies");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addRequiredParam<Real>("assembly_pitch", "Pitch [m]");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addRequiredParam<unsigned int>("nrings", "Number of fuel Pin rings per assembly [-]");
  params.addRequiredParam<Real>("flat_to_flat",
                                "Flat to flat distance for the hexagonal assembly [m]");
  params.addParam<Real>("Kij", 0.5, "Lateral form loss coefficient [-]");
  params.addRequiredParam<Real>("side_bypass",
                                "Extra size of the bypass for the side assemblies [m]");
  params.addParam<bool>(
      "tight_side_bypass",
      false,
      "Whether the side bypass shape follows the hexagonal shape of the assemblies");
  return params;
}

SCMTriInterWrapperMeshGenerator::SCMTriInterWrapperMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _unheated_length_entry(getParam<Real>("unheated_length_entry")),
    _heated_length(getParam<Real>("heated_length")),
    _unheated_length_exit(getParam<Real>("unheated_length_exit")),
    _kij(getParam<Real>("Kij")),
    _assembly_pitch(getParam<Real>("assembly_pitch")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _n_rings(getParam<unsigned int>("nrings")),
    _flat_to_flat(getParam<Real>("flat_to_flat")),
    _duct_to_pin_gap(getParam<Real>("side_bypass")),
    _tight_side_bypass(getParam<bool>("tight_side_bypass"))
{

  InterWrapperMesh::generateZGrid(
      _unheated_length_entry, _heated_length, _unheated_length_exit, _n_cells, _z_grid);

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
  TriInterWrapperMesh::rodPositions(_pin_position, _n_rings, _assembly_pitch, Point(0, 0));
  _n_assemblies = _pin_position.size();
  // assign the assemblies to the corresponding rings
  // TODO: add corner to the hexagonal assemblies
  unsigned int k = 0; // initialize the fuel assembly counter index
  _pins_in_rings.resize(_n_rings);
  _pins_in_rings[0].push_back(k++);
  for (unsigned int i = 1; i < _n_rings; i++)
    for (unsigned int j = 0; j < i * 6; j++)
      _pins_in_rings[i].push_back(k++);
  //  Given the number of pins and number of fuel Pin rings, the number of subchannels can be
  //  computed as follows:
  unsigned int chancount = 0.0;
  for (unsigned int j = 0; j < _n_rings - 1; j++)
    chancount += j * 6;
  _n_channels = chancount + _n_assemblies - 1 + (_n_rings - 1) * 6 + 6;

  // Defining the array for axial resistances
  _k_grid.resize(_n_channels, std::vector<Real>(_n_cells + 1));

  /// Re-sizing the object lists with the right number of channels
  _chan_to_pin_map.resize(_n_channels);
  _subch_type.resize(_n_channels);
  _n_gaps = _n_channels + _n_assemblies - 1; /// initial assignment
  _gap_to_chan_map.resize(_n_gaps);
  gap_fill.resize(_n_gaps);
  _chan_to_gap_map.resize(_n_channels);
  _gap_pairs_sf.resize(_n_channels);
  _chan_pairs_sf.resize(_n_channels);
  _gij_map.resize(_n_gaps);
  _sign_id_crossflow_map.resize(_n_channels);
  _gap_to_pin_map.resize(_n_gaps);
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
  } // i

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
          // corner subchannel-dummy added to hinder array size violations
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

  for (unsigned int j = 0; j < _n_gaps; j++)
  {
    if (_gap_type[j] == EChannelType::CENTER)
    {
      _gij_map[j] = _assembly_pitch - _flat_to_flat;
    }
    else if (_gap_type[j] == EChannelType::EDGE || _gap_type[j] == EChannelType::CORNER)
    {
      _gij_map[j] = _duct_to_pin_gap;
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
        if (_tight_side_bypass)
          a1 = _flat_to_flat * std::tan(libMesh::pi / 6.0) / 2.0 + _duct_to_pin_gap / 2.0;
        else
          a1 = _flat_to_flat / 2.0 + _duct_to_pin_gap / 2.0;
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
      a1 = _flat_to_flat / 2.0 + _duct_to_pin_gap / 2.0;
      a2 = std::sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) + a1;
      _subchannel_position[i][0] = (a2 * x1 - a1 * x0) / (a2 - a1);
      _subchannel_position[i][1] = (a2 * y1 - a1 * y0) / (a2 - a1);
    }
  } // i
  // Reduce reserved memory in the channel-to-gap map.
  for (auto & gap : _chan_to_gap_map)
  {
    gap.shrink_to_fit();
  }
}

std::unique_ptr<MeshBase>
SCMTriInterWrapperMeshGenerator::generate()
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

  mesh_base->prepare_for_use();

  // move the meta data into TriInterWrapperMesh
  auto & sch_mesh = static_cast<TriInterWrapperMesh &>(*_mesh);
  sch_mesh._unheated_length_entry = _unheated_length_entry;
  sch_mesh._heated_length = _heated_length;
  sch_mesh._unheated_length_exit = _unheated_length_exit;
  sch_mesh._z_grid = _z_grid;
  sch_mesh._k_grid = _k_grid;
  sch_mesh._kij = _kij;
  sch_mesh._assembly_pitch = _assembly_pitch;
  sch_mesh._n_cells = _n_cells;
  sch_mesh._n_rings = _n_rings;
  sch_mesh._n_channels = _n_channels;
  sch_mesh._flat_to_flat = _flat_to_flat;
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
  sch_mesh._n_assemblies = _n_assemblies;
  sch_mesh._n_gaps = _n_gaps;
  sch_mesh._subch_type = _subch_type;
  sch_mesh._gap_type = _gap_type;
  sch_mesh._gap_pairs_sf = _gap_pairs_sf;
  sch_mesh._chan_pairs_sf = _chan_pairs_sf;
  sch_mesh._tight_side_bypass = _tight_side_bypass;

  // Overloading assembly sides with flat_to_flat distance
  sch_mesh._assembly_side_x = _flat_to_flat;
  sch_mesh._assembly_side_y = _flat_to_flat;

  return mesh_base;
}
