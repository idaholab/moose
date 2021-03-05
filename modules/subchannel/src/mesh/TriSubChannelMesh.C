#include "TriSubChannelMesh.h"

#include <cmath>

#include "libmesh/edge_edge2.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", TriSubChannelMesh);

InputParameters
TriSubChannelMesh::validParams()
{
  InputParameters params = SubChannelMeshBase::validParams();
  params.addRequiredParam<unsigned int>("nrings", "Number of fuel rod rings per assembly [-]");
  params.addRequiredParam<Real>("flat_to_flat",
                                "Flat to flat distance for the hexagonal assembly [m]");
  return params;
}

void
triRodPositions(std::vector<Point> & positions, unsigned int nrings, Real pitch, Point center)
{
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

TriSubChannelMesh::TriSubChannelMesh(const InputParameters & params)
  : SubChannelMeshBase(params),
    _nrings(getParam<unsigned int>("nrings")),
    _flat_to_flat(getParam<Real>("flat_to_flat")),
    _duct_to_rod_gap(0.5 *
                     (_flat_to_flat - (_nrings - 1) * _pitch * std::sqrt(3.0) - _rod_diameter))
{
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

  triRodPositions(_rod_position, _nrings, _pitch, Point(0, 0));
  _nrods = _rod_position.size();

  // assign the rods to the corresponding rings
  unsigned int k = 0; // initializat the fuel rod counter index
  _rods_in_rings.resize(_nrings);
  _rods_in_rings[0].push_back(k++);
  for (unsigned int i = 1; i < _nrings; i++)
    for (unsigned int j = 0; j < i * 6; j++)
      _rods_in_rings[i].push_back(k++);

  // to find the total number of subchannels...
  // inner ring subchannels = 6 ,
  // for each ring starting with 3rd ring hex corner rods and middle rods can be counted as 1 and 2
  // subchannels. (_nrings-2)*12 + _nrods - 7 - (_nrings-2)*6
  //  there are 6 corner subchannels and number of edge subchannels is  2*((_nrings-1)*6 - 6)
  _n_channels =
      6 + (_nrings - 2) * 12 + _nrods - 7 - (_nrings - 2) * 6 + 6 + 2 * ((_nrings - 1) * 6 - 6);
  _subchannel_to_rod_map.resize(_n_channels);
  _subch_type.resize(_n_channels);
  _n_gaps = _n_channels + _nrods - 1; /// initial assignment
  _gap_to_chan_map.resize(_n_gaps);
  _chan_to_gap_map.resize(_n_channels);
  _gij_map.resize(_n_gaps);
  _sign_id_crossflow_map.resize(_n_channels);
  _gap_to_rod_map.resize(_n_gaps);
  _gap_type.resize(_n_gaps);
  _subchannel_position.resize(_n_channels);
  for (unsigned int i = 0; i < _n_gaps; i++)
  {
    _gap_to_rod_map[i].reserve(2);
  } // i

  for (unsigned int i = 0; i < _n_channels; i++)
  {
    _subchannel_to_rod_map[i].reserve(3);
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
  // for each ring we trace the subchannels by pairing up to neighbor rods and looking for the third
  // rod at inner or outer ring compared to the current ring.
  for (unsigned int i = 1; i < _nrings; i++)
  {
    // find the closest rod at back ring
    for (unsigned int j = 0; j < _rods_in_rings[i].size(); j++)
    {
      if (j == _rods_in_rings[i].size() - 1)
      {
        _subchannel_to_rod_map[k].push_back(_rods_in_rings[i][j]);
        _subchannel_to_rod_map[k].push_back(_rods_in_rings[i][0]);
        avg_coor_x =
            0.5 * (_rod_position[_rods_in_rings[i][j]](0) + _rod_position[_rods_in_rings[i][0]](0));
        avg_coor_y =
            0.5 * (_rod_position[_rods_in_rings[i][j]](1) + _rod_position[_rods_in_rings[i][0]](1));
        _gap_to_rod_map[kgap].push_back(_rods_in_rings[i][0]);
        _gap_to_rod_map[kgap].push_back(_rods_in_rings[i][j]);
        _gap_type[kgap] = EChannelType::CENTER;
        kgap = kgap + 1;
      }
      else
      {
        _subchannel_to_rod_map[k].push_back(_rods_in_rings[i][j]);
        _subchannel_to_rod_map[k].push_back(_rods_in_rings[i][j + 1]);
        avg_coor_x = 0.5 * (_rod_position[_rods_in_rings[i][j]](0) +
                            _rod_position[_rods_in_rings[i][j + 1]](0));
        avg_coor_y = 0.5 * (_rod_position[_rods_in_rings[i][j]](1) +
                            _rod_position[_rods_in_rings[i][j + 1]](1));
        _gap_to_rod_map[kgap].push_back(_rods_in_rings[i][j]);
        _gap_to_rod_map[kgap].push_back(_rods_in_rings[i][j + 1]);
        _gap_type[kgap] = EChannelType::CENTER;
        kgap = kgap + 1;
      }

      dist0 = 1.0e+5;

      _subchannel_to_rod_map[k].push_back(_rods_in_rings[i - 1][0]);
      unsigned int l0 = 0;

      for (unsigned int l = 0; l < _rods_in_rings[i - 1].size(); l++)
      {
        dist = std::sqrt(pow(_rod_position[_rods_in_rings[i - 1][l]](0) - avg_coor_x, 2) +
                         pow(_rod_position[_rods_in_rings[i - 1][l]](1) - avg_coor_y, 2));

        if (dist < dist0)
        {
          _subchannel_to_rod_map[k][2] = _rods_in_rings[i - 1][l];
          l0 = l;
          dist0 = dist;
        } // if
      }   // l

      _gap_to_rod_map[kgap].push_back(_rods_in_rings[i][j]);
      _gap_to_rod_map[kgap].push_back(_rods_in_rings[i - 1][l0]);
      _gap_type[kgap] = EChannelType::CENTER;
      kgap = kgap + 1;
      _subch_type[k] = EChannelType::CENTER;
      k = k + 1;

    } // for j

    // find the closest rod at front ring

    for (unsigned int j = 0; j < _rods_in_rings[i].size(); j++)
    {
      if (j == _rods_in_rings[i].size() - 1)
      {
        _subchannel_to_rod_map[k].push_back(_rods_in_rings[i][j]);
        _subchannel_to_rod_map[k].push_back(_rods_in_rings[i][0]);
        avg_coor_x =
            0.5 * (_rod_position[_rods_in_rings[i][j]](0) + _rod_position[_rods_in_rings[i][0]](0));
        avg_coor_y =
            0.5 * (_rod_position[_rods_in_rings[i][j]](1) + _rod_position[_rods_in_rings[i][0]](1));
      }
      else
      {
        _subchannel_to_rod_map[k].push_back(_rods_in_rings[i][j]);
        _subchannel_to_rod_map[k].push_back(_rods_in_rings[i][j + 1]);
        avg_coor_x = 0.5 * (_rod_position[_rods_in_rings[i][j]](0) +
                            _rod_position[_rods_in_rings[i][j + 1]](0));
        avg_coor_y = 0.5 * (_rod_position[_rods_in_rings[i][j]](1) +
                            _rod_position[_rods_in_rings[i][j + 1]](1));
      }

      // if the outermost ring, set the edge subchannels first... then the corner subchannels
      if (i == _nrings - 1)
      {
        // add  edges
        _subch_type[k] = EChannelType::EDGE; // an edge subchannel is created
        _gap_to_rod_map[kgap].push_back(_rods_in_rings[i][j]);
        _gap_to_rod_map[kgap].push_back(_rods_in_rings[i][j]);
        _gap_type[kgap] = EChannelType::EDGE;
        _chan_to_gap_map[k].push_back(kgap);
        kgap = kgap + 1;
        k = k + 1;

        if (j % i == 0)
        {
          // generate a corner subchannel, generate the additional gap and fix chan_to_gap_map
          _gap_to_rod_map[kgap].push_back(_rods_in_rings[i][j]);
          _gap_to_rod_map[kgap].push_back(_rods_in_rings[i][j]);
          _gap_type[kgap] = EChannelType::CORNER;

          // corner subchannel
          _subchannel_to_rod_map[k].push_back(_rods_in_rings[i][j]);
          // corner subchannel-dummy added to hinder array size violations
          _subchannel_to_rod_map[k].push_back(_rods_in_rings[i][j]);
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
        _subchannel_to_rod_map[k].push_back(_rods_in_rings[i + 1][0]);
        for (unsigned int l = 0; l < _rods_in_rings[i + 1].size(); l++)
        {
          dist = std::sqrt(pow(_rod_position[_rods_in_rings[i + 1][l]](0) - avg_coor_x, 2) +
                           pow(_rod_position[_rods_in_rings[i + 1][l]](1) - avg_coor_y, 2));
          if (dist < dist0)
          {
            _subchannel_to_rod_map[k][2] = _rods_in_rings[i + 1][l];
            dist0 = dist;
            l0 = l;
          } // if
        }   // l

        _gap_to_rod_map[kgap].push_back(_rods_in_rings[i][j]);
        _gap_to_rod_map[kgap].push_back(_rods_in_rings[i + 1][l0]);
        _gap_type[kgap] = EChannelType::CENTER;
        kgap = kgap + 1;
        _subch_type[k] = EChannelType::CENTER;
        k = k + 1;
      } // if
    }   // for j
  }     // for i

  // find the _gap_to_chan_map and _chan_to_gap_map using the gap_to_rod and subchannel_to_rod_maps

  for (unsigned int i = 0; i < _n_channels; i++)
  {
    if (_subch_type[i] == EChannelType::CENTER)
    {
      for (unsigned int j = 0; j < _n_gaps; j++)
      {
        if (_gap_type[j] == EChannelType::CENTER)
        {
          if (((_subchannel_to_rod_map[i][0] == _gap_to_rod_map[j][0]) &&
               (_subchannel_to_rod_map[i][1] == _gap_to_rod_map[j][1])) ||
              ((_subchannel_to_rod_map[i][0] == _gap_to_rod_map[j][1]) &&
               (_subchannel_to_rod_map[i][1] == _gap_to_rod_map[j][0])))
          {
            _chan_to_gap_map[i].push_back(j);
          }

          if (((_subchannel_to_rod_map[i][0] == _gap_to_rod_map[j][0]) &&
               (_subchannel_to_rod_map[i][2] == _gap_to_rod_map[j][1])) ||
              ((_subchannel_to_rod_map[i][0] == _gap_to_rod_map[j][1]) &&
               (_subchannel_to_rod_map[i][2] == _gap_to_rod_map[j][0])))
          {
            _chan_to_gap_map[i].push_back(j);
          }

          if (((_subchannel_to_rod_map[i][1] == _gap_to_rod_map[j][0]) &&
               (_subchannel_to_rod_map[i][2] == _gap_to_rod_map[j][1])) ||
              ((_subchannel_to_rod_map[i][1] == _gap_to_rod_map[j][1]) &&
               (_subchannel_to_rod_map[i][2] == _gap_to_rod_map[j][0])))
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
          if (((_subchannel_to_rod_map[i][0] == _gap_to_rod_map[j][0]) &&
               (_subchannel_to_rod_map[i][1] == _gap_to_rod_map[j][1])) ||
              ((_subchannel_to_rod_map[i][0] == _gap_to_rod_map[j][1]) &&
               (_subchannel_to_rod_map[i][1] == _gap_to_rod_map[j][0])))
          {
            _chan_to_gap_map[i].push_back(j);
          }
        }
      } // for j

      // assign the second/front gap of the edge subchannel. If the next rod is corner, you must
      // take _gap_to_rod_map[j][2] = 2 gap first figure out if the front rod is a corner rod
      icorner = 0;
      for (unsigned int k = 0; k < _n_channels; k++)
      {
        if (_subch_type[k] == EChannelType::CORNER &&
            _subchannel_to_rod_map[i][1] == _subchannel_to_rod_map[k][0])
        {
          _chan_to_gap_map[i].push_back(_chan_to_gap_map[k][1]);
          icorner = 1;
          break;
        } // if
      }   // for

      for (unsigned int k = 0; k < _n_channels; k++)
      {
        if (_subch_type[k] == EChannelType::CORNER &&
            _subchannel_to_rod_map[i][0] == _subchannel_to_rod_map[k][0])
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
          if (_gap_to_chan_map[j].first == 0)
          {
            _gap_to_chan_map[j].first = i;
          }
          else if (_gap_to_chan_map[j].second == 0)
          {
            _gap_to_chan_map[j].second = i;
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
          if (_gap_to_chan_map[j].first == 0)
          {
            _gap_to_chan_map[j].first = i;
          }
          else if (_gap_to_chan_map[j].second == 0)
          {
            _gap_to_chan_map[j].second = i;
          }
          else
          {
          }
        }
      }
    } // i
  }   // j

  // set the _gij_map

  for (unsigned int j = 0; j < _n_gaps; j++)
  {
    if (_gap_type[j] == EChannelType::CENTER)
    {
      _gij_map[j] = _pitch - _rod_diameter;
    }
    else if (_gap_type[j] == EChannelType::EDGE || _gap_type[j] == EChannelType::CORNER)
    {
      _gij_map[j] = _duct_to_rod_gap;
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
      }   // k
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
      }   // k
    }     // subch_type =2
  }       // i

  // set the subchannel positions
  for (unsigned int i = 0; i < _n_channels; i++)
  {
    if (_subch_type[i] == EChannelType::CENTER)
    {
      _subchannel_position[i][0] = (_rod_position[_subchannel_to_rod_map[i][0]](0) +
                                    _rod_position[_subchannel_to_rod_map[i][1]](0) +
                                    _rod_position[_subchannel_to_rod_map[i][2]](0)) /
                                   3.0;
      _subchannel_position[i][1] = (_rod_position[_subchannel_to_rod_map[i][0]](1) +
                                    _rod_position[_subchannel_to_rod_map[i][1]](1) +
                                    _rod_position[_subchannel_to_rod_map[i][2]](1)) /
                                   3.0;
    }
    else if (_subch_type[i] == EChannelType::EDGE)
    {
      for (unsigned int j = 0; j < _n_channels; j++)
      {
        if (_subch_type[j] == EChannelType::CENTER &&
            ((_subchannel_to_rod_map[i][0] == _subchannel_to_rod_map[j][0] &&
              _subchannel_to_rod_map[i][1] == _subchannel_to_rod_map[j][1]) ||
             (_subchannel_to_rod_map[i][0] == _subchannel_to_rod_map[j][1] &&
              _subchannel_to_rod_map[i][1] == _subchannel_to_rod_map[j][0])))
        {
          x0 = _rod_position[_subchannel_to_rod_map[j][2]](0);
          y0 = _rod_position[_subchannel_to_rod_map[j][2]](1);
        }
        else if (_subch_type[j] == EChannelType::CENTER &&
                 ((_subchannel_to_rod_map[i][0] == _subchannel_to_rod_map[j][0] &&
                   _subchannel_to_rod_map[i][1] == _subchannel_to_rod_map[j][2]) ||
                  (_subchannel_to_rod_map[i][0] == _subchannel_to_rod_map[j][2] &&
                   _subchannel_to_rod_map[i][1] == _subchannel_to_rod_map[j][0])))
        {
          x0 = _rod_position[_subchannel_to_rod_map[j][1]](0);
          y0 = _rod_position[_subchannel_to_rod_map[j][1]](1);
        }
        else if (_subch_type[j] == EChannelType::CENTER &&
                 ((_subchannel_to_rod_map[i][0] == _subchannel_to_rod_map[j][1] &&
                   _subchannel_to_rod_map[i][1] == _subchannel_to_rod_map[j][2]) ||
                  (_subchannel_to_rod_map[i][0] == _subchannel_to_rod_map[j][2] &&
                   _subchannel_to_rod_map[i][1] == _subchannel_to_rod_map[j][1])))
        {
          x0 = _rod_position[_subchannel_to_rod_map[j][0]](0);
          y0 = _rod_position[_subchannel_to_rod_map[j][0]](1);
        }
        x1 = 0.5 * (_rod_position[_subchannel_to_rod_map[i][0]](0) +
                    _rod_position[_subchannel_to_rod_map[i][1]](0));
        y1 = 0.5 * (_rod_position[_subchannel_to_rod_map[i][0]](1) +
                    _rod_position[_subchannel_to_rod_map[i][1]](1));
        a1 = _rod_diameter / 2.0 + _duct_to_rod_gap / 2.0;
        a2 = std::sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) + a1;
        _subchannel_position[i][0] = (a2 * x1 - a1 * x0) / (a2 - a1);
        _subchannel_position[i][1] = (a2 * y1 - a1 * y0) / (a2 - a1);
      } // j
    }
    else if (_subch_type[i] == EChannelType::CORNER)
    {
      x0 = _rod_position[0](0);
      y0 = _rod_position[0](1);
      x1 = _rod_position[_subchannel_to_rod_map[i][0]](0);
      y1 = _rod_position[_subchannel_to_rod_map[i][0]](1);
      a1 = _rod_diameter / 2.0 + _duct_to_rod_gap / 2.0;
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

TriSubChannelMesh::TriSubChannelMesh(const TriSubChannelMesh & other_mesh)
  : SubChannelMeshBase(other_mesh),
    _n_channels(other_mesh._n_channels),
    _nodes(other_mesh._nodes),
    _gap_to_chan_map(other_mesh._gap_to_chan_map),
    _chan_to_gap_map(other_mesh._chan_to_gap_map),
    _sign_id_crossflow_map(other_mesh._sign_id_crossflow_map),
    _gij_map(other_mesh._gij_map),
    _subchannel_position(other_mesh._subchannel_position)
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

void
TriSubChannelMesh::buildMesh()
{
  // Generate nodes and elements given the subchannel and subchannel x, y positions and z_grid
  // Set the boundary nodes

  UnstructuredMesh & mesh = dynamic_cast<UnstructuredMesh &>(getMesh());
  mesh.clear();
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  mesh.set_spatial_dimension(3);
  mesh.reserve_elem(_nz * _n_channels);
  mesh.reserve_nodes((_nz + 1) * _n_channels);
  _nodes.resize(_n_channels);
  // Add the points for the give x,y subchannel positions.  The grid is hexagonal.
  //  The grid along
  // z is irregular to account for rod spacers.  Store pointers in the _nodes
  // array so we can keep track of which points are in which channels.
  unsigned int node_id = 0;
  for (unsigned int i = 0; i < _n_channels; i++)
  {
    _nodes[i].reserve(_nz + 1);
    for (unsigned int iz = 0; iz < _nz + 1; iz++)
    {
      _nodes[i].push_back(mesh.add_point(
          Point(_subchannel_position[i][0], _subchannel_position[i][1], _z_grid[iz]), node_id++));
    }
  }

  // Add the elements which in this case are 2-node edges that link each
  // subchannel's nodes vertically.
  unsigned int elem_id = 0;
  for (unsigned int i = 0; i < _n_channels; i++)
  {
    for (unsigned int iz = 0; iz < _nz; iz++)
    {
      Elem * elem = new Edge2;
      elem->set_id(elem_id++);
      elem = mesh.add_elem(elem);
      const int indx1 = (_nz + 1) * i + iz;
      const int indx2 = (_nz + 1) * i + (iz + 1);
      elem->set_node(0) = mesh.node_ptr(indx1);
      elem->set_node(1) = mesh.node_ptr(indx2);

      if (iz == 0)
        boundary_info.add_side(elem, 0, 0);
      if (iz == _nz - 1)
        boundary_info.add_side(elem, 1, 1);
    }
  }
  boundary_info.sideset_name(0) = "inlet";
  boundary_info.sideset_name(1) = "outlet";
  boundary_info.nodeset_name(0) = "inlet";
  boundary_info.nodeset_name(1) = "outlet";
  mesh.prepare_for_use();
}
