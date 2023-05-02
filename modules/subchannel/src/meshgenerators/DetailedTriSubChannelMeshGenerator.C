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

#include "DetailedTriSubChannelMeshGenerator.h"
#include "TriSubChannelMesh.h"
#include <array>
#include <cmath>
#include "libmesh/cell_prism6.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", DetailedTriSubChannelMeshGenerator);

InputParameters
DetailedTriSubChannelMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Creates detailed mesh of subchannels in a triangular lattice arrangement");
  params.addRequiredParam<Real>("pitch", "Pitch [m]");
  params.addRequiredParam<Real>("rod_diameter", "Rod diameter [m]");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addRequiredParam<unsigned int>("nrings", "Number of fuel rod rings per assembly [-]");
  params.addRequiredParam<Real>("flat_to_flat",
                                "Flat to flat distance for the hexagonal assembly [m]");
  params.addParam<unsigned int>("block_id", 0, "Block ID used for the mesh subdomain.");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addParam<bool>("verbose_flag", false, "Flag to print out the mesh coordinates.");
  return params;
}

DetailedTriSubChannelMeshGenerator::DetailedTriSubChannelMeshGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    _unheated_length_entry(getParam<Real>("unheated_length_entry")),
    _heated_length(getParam<Real>("heated_length")),
    _unheated_length_exit(getParam<Real>("unheated_length_exit")),
    _pitch(getParam<Real>("pitch")),
    _rod_diameter(getParam<Real>("rod_diameter")),
    _n_rings(getParam<unsigned int>("nrings")),
    _flat_to_flat(getParam<Real>("flat_to_flat")),
    _block_id(getParam<unsigned int>("block_id")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _verbose(getParam<bool>("verbose_flag"))
{
  Real L = _unheated_length_entry + _heated_length + _unheated_length_exit;
  Real dz = L / _n_cells;
  for (unsigned int i = 0; i < _n_cells + 1; i++)
    _z_grid.push_back(dz * i);

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
  // the indicator used while setting _gap_to_chan_map array
  std::vector<std::pair<unsigned int, unsigned int>> gap_fill;
  TriSubChannelMesh::rodPositions(_rod_position, _n_rings, _pitch, Point(0, 0));
  _nrods = _rod_position.size();
  // assign the rods to the corresponding rings
  unsigned int k = 0; // initialize the fuel rod counter index
  _rods_in_rings.resize(_n_rings);
  _rods_in_rings[0].push_back(k++);
  for (unsigned int i = 1; i < _n_rings; i++)
    for (unsigned int j = 0; j < i * 6; j++)
      _rods_in_rings[i].push_back(k++);
  //  Given the number of rods and number of fuel rod rings, the number of subchannels can be
  //  computed as follows:
  unsigned int chancount = 0.0;
  // Summing internal channels
  for (unsigned int j = 0; j < _n_rings - 1; j++)
    chancount += j * 6;
  // Adding external channels to the total count
  _n_channels = chancount + _nrods - 1 + (_n_rings - 1) * 6 + 6;

  // Utils for building the mesh
  _subchannel_to_rod_map.resize(_n_channels);
  _subch_type.resize(_n_channels);
  _subchannel_position.resize(_n_channels);

  for (unsigned int i = 0; i < _n_channels; i++)
  {
    _subchannel_to_rod_map[i].reserve(3);
    _subchannel_position[i].reserve(3);
    for (unsigned int j = 0; j < 3; j++)
    {
      _subchannel_position.at(i).push_back(0.0);
    }
  } // i

  // create the subchannels
  k = 0; // initialize the subchannel counter index
  for (unsigned int i = 1; i < _n_rings; i++)
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

      dist0 = 1.0e+5;

      _subchannel_to_rod_map[k].push_back(_rods_in_rings[i - 1][0]);

      for (unsigned int l = 0; l < _rods_in_rings[i - 1].size(); l++)
      {
        dist = std::sqrt(pow(_rod_position[_rods_in_rings[i - 1][l]](0) - avg_coor_x, 2) +
                         pow(_rod_position[_rods_in_rings[i - 1][l]](1) - avg_coor_y, 2));

        if (dist < dist0)
        {
          _subchannel_to_rod_map[k][2] = _rods_in_rings[i - 1][l];
          dist0 = dist;
        } // if
      }   // l

      _subch_type[k] = EChannelType::CENTER;
      _orientation_map.insert(std::make_pair(k, 0.0));
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
      if (i == _n_rings - 1)
      {
        // add  edges
        _subch_type[k] = EChannelType::EDGE; // an edge subchannel is created
        k = k + 1;

        if (j % i == 0)
        {

          // corner subchannel
          _subchannel_to_rod_map[k].push_back(_rods_in_rings[i][j]);
          // corner subchannel-dummy added to hinder array size violations
          _subchannel_to_rod_map[k].push_back(_rods_in_rings[i][j]);
          _subch_type[k] = EChannelType::CORNER;

          k = k + 1;
        }
        // if not the outer most ring
      }
      else
      {
        dist0 = 1.0e+5;
        _subchannel_to_rod_map[k].push_back(_rods_in_rings[i + 1][0]);
        for (unsigned int l = 0; l < _rods_in_rings[i + 1].size(); l++)
        {
          dist = std::sqrt(pow(_rod_position[_rods_in_rings[i + 1][l]](0) - avg_coor_x, 2) +
                           pow(_rod_position[_rods_in_rings[i + 1][l]](1) - avg_coor_y, 2));
          if (dist < dist0)
          {
            _subchannel_to_rod_map[k][2] = _rods_in_rings[i + 1][l];
            dist0 = dist;
          } // if
        }   // l

        _subch_type[k] = EChannelType::CENTER;
        _orientation_map.insert(std::make_pair(k, libMesh::pi));
        k = k + 1;
      } // if
    }   // for j
  }     // for i

  // set the subchannel positions
  Real _duct_to_rod_gap =
      0.5 * (_flat_to_flat - (_n_rings - 1) * _pitch * std::sqrt(3.0) - _rod_diameter);
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
  }
}

std::unique_ptr<MeshBase>
DetailedTriSubChannelMeshGenerator::generate()
{
  auto mesh_base = buildMeshBaseObject();
  BoundaryInfo & boundary_info = mesh_base->get_boundary_info();
  mesh_base->set_spatial_dimension(3);
  // Define the resolution (the number of points used to represent a circle).
  // This must be divisible by 4.
  const unsigned int theta_res_triangle = 24; // TODO: parameterize
  const unsigned int theta_res_square = 24;   // TODO: parameterize
  // Compute the number of points needed to represent one sixth and quadrant of a circle.
  const unsigned int points_per_sixth = theta_res_triangle / 6 + 1;
  const unsigned int points_per_quadrant = theta_res_square / 4 + 1;

  // Compute the points needed to represent one axial cross-flow of a subchannel.
  // For the center subchannel (sc) there is one center point plus the points from 3 side
  // circles.
  const unsigned int points_per_center = points_per_sixth * 3 + 1;
  // For the corner sc there is one center point plus the points from 1 side circle plus 3
  // corners
  const unsigned int points_per_corner = points_per_sixth * 1 + 1 + 3;
  // For the side sc there is one center point plus the points from 2 intersecting circles plus 2
  // corners
  const unsigned int points_per_side = points_per_quadrant * 2 + 1 + 2;

  if (_verbose)
  {
    _console << "Points per center: " << points_per_center << std::endl;
    _console << "Points per side: " << points_per_side << std::endl;
    _console << "Points per corner: " << points_per_corner << std::endl;
  }

  // Compute the number of elements (Prism6) which combined base creates the sub-channel
  // cross-section
  const unsigned int elems_per_center = theta_res_triangle * 3 / 6 + 3; // TODO: check
  const unsigned int elems_per_corner = theta_res_triangle / 6 + 4;
  const unsigned int elems_per_side = 2 * theta_res_square / 4 + 4;
  if (_verbose)
  {
    _console << "Elems per center: " << elems_per_center << std::endl;
    _console << "Elems per side: " << elems_per_side << std::endl;
    _console << "Elems per corner: " << elems_per_corner << std::endl;
  }

  // specify number and type of sub-channel
  unsigned int n_center, n_side, n_corner;
  if (_n_rings == 1)
  {
    n_corner = 6;
    n_side = 0;
    n_center = _n_channels - n_side - n_corner;
  }
  else
  {
    n_corner = 6;
    n_side = (_n_rings - 1) * 6;
    n_center = _n_channels - n_side - n_corner;
  }
  if (_verbose)
  {
    _console << "Centers: " << n_center << std::endl;
    _console << "Sides: " << n_side << std::endl;
    _console << "Corners: " << n_corner << std::endl;
  }

  // Compute the total number of points and elements.
  const unsigned int points_per_level =
      n_corner * points_per_corner + n_side * points_per_side + n_center * points_per_center;
  const unsigned int elems_per_level =
      n_corner * elems_per_corner + n_side * elems_per_side + n_center * elems_per_center;
  if (_verbose)
  {
    _console << "Points per level: " << points_per_level << std::endl;
    _console << "Elements per level: " << elems_per_level << std::endl;
  }
  const unsigned int n_points = points_per_level * (_n_cells + 1);
  const unsigned int n_elems = elems_per_level * _n_cells;
  if (_verbose)
  {
    _console << "Number of points: " << n_points << std::endl;
    _console << "Number of elements: " << n_elems << std::endl;
  }
  mesh_base->reserve_nodes(n_points);
  mesh_base->reserve_elem(n_elems);
  // Build an array of points arranged in a circle on the xy-plane. (last and first node overlap)
  // We build for both the square discretization in the edges and the triangular discretization
  // within the mesh
  const double radius = _rod_diameter / 2.0;
  std::array<Point, theta_res_square + 1> circle_points_square;
  {
    double theta = 0;
    for (unsigned int i = 0; i < theta_res_square + 1; i++)
    {
      circle_points_square[i](0) = radius * std::cos(theta);
      circle_points_square[i](1) = radius * std::sin(theta);
      theta += 2.0 * libMesh::pi / theta_res_square;
    }
  }
  std::array<Point, theta_res_triangle + 1> circle_points_triangle;
  {
    double theta = 0;
    for (unsigned int i = 0; i < theta_res_triangle + 1; i++)
    {
      circle_points_triangle[i](0) = radius * std::cos(theta);
      circle_points_triangle[i](1) = radius * std::sin(theta);
      theta += 2.0 * libMesh::pi / theta_res_triangle;
    }
  }
  // Define "quadrant center" reference points.  These will be the centers of
  // the 3 circles that represent the fuel rods.  These centers are
  // offset a little bit so that in the final mesh, there is a tiny gap between
  // neighboring subchannel cells.  That allows us to easily map a solution to
  // this detailed mesh with a nearest-neighbor search.
  const Real shrink_factor = 0.99999;
  // Quadrants are used only for the side and corner subchannels
  Real _duct_to_rod_gap =
      0.5 * (_flat_to_flat - (_n_rings - 1) * _pitch * std::sqrt(3.0) - _rod_diameter);
  std::array<Point, 2> quadrant_centers_sides;
  quadrant_centers_sides[0] = Point(
      -_pitch * 0.5 * shrink_factor, -(_duct_to_rod_gap + _rod_diameter) * 0.5 * shrink_factor, 0);
  quadrant_centers_sides[1] = Point(
      _pitch * 0.5 * shrink_factor, -(_duct_to_rod_gap + _rod_diameter) * 0.5 * shrink_factor, 0);
  std::array<Point, 1> quadrant_centers_corner;
  quadrant_centers_corner[0] =
      Point(-(_duct_to_rod_gap + _rod_diameter) * 0.5 * std::sin(libMesh::pi / 6) * shrink_factor,
            -(_duct_to_rod_gap + _rod_diameter) * 0.5 * std::cos(libMesh::pi / 6) * shrink_factor,
            0);
  // Triangles are used for all center subchannels
  std::array<Point, 3> triangle_centers;
  triangle_centers[0] = Point(0, _pitch * std::cos(libMesh::pi / 6) * 2 / 3 * shrink_factor, 0);
  triangle_centers[1] = Point(-_pitch * 0.5 * shrink_factor,
                              -_pitch * std::cos(libMesh::pi / 6) * 1 / 3 * shrink_factor,
                              0);
  triangle_centers[2] = Point(
      _pitch * 0.5 * shrink_factor, -_pitch * std::cos(libMesh::pi / 6) * 1 / 3 * shrink_factor, 0);

  const unsigned int m_sixth = theta_res_triangle / 6;
  const unsigned int m_quarter = theta_res_square / 4;
  // Build an array of points that represent a cross section of a center subchannel
  // cell.  The points are ordered in this fashion:
  //    3   1
  //      2
  //      0
  // 4 5         8 9
  //  6         7
  std::array<Point, points_per_center> center_points;
  {
    unsigned int start;
    for (unsigned int i = 0; i < 3; i++)
    {
      if (i == 0)
        start = 5 * (m_sixth);
      if (i == 1)
        start = 1 * (m_sixth);
      if (i == 2)
        start = 3 * (m_sixth);
      for (unsigned int ii = 0; ii < points_per_sixth; ii++)
      {
        auto c_pt = circle_points_triangle[start - ii];
        center_points[i * points_per_sixth + ii + 1] = triangle_centers[i] + c_pt;
      }
    }
  }

  // Build an array of points that represent a cross section of a top left corner subchannel
  // cell. The points are ordered in this fashion:
  // 6
  //        5
  //    0
  // 1 2         4
  //   3
  std::array<Point, points_per_corner> corner_points;
  {
    for (unsigned int ii = 0; ii < points_per_sixth; ii++)
    {
      auto c_pt = circle_points_triangle[1 * m_quarter - ii];
      corner_points[ii + 1] = quadrant_centers_corner[0] + c_pt;
    }
    Real side_short = (_duct_to_rod_gap + _rod_diameter) * 0.5;
    Real side_long = (2.0 * _duct_to_rod_gap + _rod_diameter) * 0.5;
    Real side_length = std::sqrt(std::pow(side_short, 2) + std::pow(side_long, 2) -
                                 2 * side_short * side_long * std::cos(libMesh::pi / 6));
    Real angle =
        libMesh::pi - libMesh::pi / 3 -
        std::acos((-std::pow(side_long, 2) + std::pow(side_short, 2) + std::pow(side_length, 2)) /
                  (2 * side_short * side_length));
    corner_points[points_per_sixth + 1] = Point(side_length * std::cos(angle) * shrink_factor,
                                                -side_length * std::sin(angle) * shrink_factor,
                                                0);
    corner_points[points_per_sixth + 2] =
        Point(0.5 * _duct_to_rod_gap * shrink_factor * std::tan(libMesh::pi / 6),
              0.5 * _duct_to_rod_gap * shrink_factor / std::cos(libMesh::pi / 6),
              0);
    corner_points[points_per_sixth + 3] =
        Point(-side_length * std::cos(libMesh::pi / 2 - angle - libMesh::pi / 6) * shrink_factor,
              side_length * std::sin(libMesh::pi / 2 - angle - libMesh::pi / 6) * shrink_factor,
              0);
  }

  // Build an array of points that represent a cross-section of a top side subchannel
  // cell.  The points are ordered in this fashion:
  // 8            7
  //
  //       0
  // 1 2        5 6
  //    3     4
  std::array<Point, points_per_side> side_points;
  {
    for (unsigned int ii = 0; ii < points_per_quadrant; ii++)
    {
      auto c_pt = circle_points_square[m_quarter - ii];
      side_points[ii + 1] = quadrant_centers_sides[0] + c_pt;
    }
    for (unsigned int ii = 0; ii < points_per_quadrant; ii++)
    {
      auto c_pt = circle_points_square[2 * m_quarter - ii];
      side_points[points_per_quadrant + ii + 1] = quadrant_centers_sides[1] + c_pt;
    }
    side_points[2 * points_per_quadrant + 1] =
        Point(_pitch * 0.5 * shrink_factor, 0.5 * _duct_to_rod_gap * shrink_factor, 0);
    side_points[2 * points_per_quadrant + 2] =
        Point(-_pitch * 0.5 * shrink_factor, 0.5 * _duct_to_rod_gap * shrink_factor, 0);
  }

  int point_counter = 0;
  unsigned int node_id = 0;
  for (unsigned int i = 0; i < _n_channels; i++)
  {
    //    Real offset_x = _flat_to_flat / 2.0;
    //    Real offset_y = _flat_to_flat / 2.0;

    if (getSubchannelType(i) == EChannelType::CENTER)
    {
      for (auto z : _z_grid)
      {
        // Get height
        Point z0{0, 0, z};

        // Get suchannel position and assign to point
        auto loc_position = getSubchannelPosition(i);
        Point p0{loc_position[0], loc_position[1], 0};

        // Determine orientation of current subchannel
        auto subchannel_rods = getSubChannelRods(i);
        Point subchannel_side =
            getRodPosition(subchannel_rods[0]) + getRodPosition(subchannel_rods[1]);
        Point base_center_orientation = {0, -1};

        // Get rotation angle for current subchannel
        Real dot_prod = 0;
        for (unsigned int lp = 0; lp < 2; lp++)
          dot_prod += base_center_orientation(lp) * subchannel_side(lp);
        auto theta =
            std::acos(dot_prod / (base_center_orientation.norm() * subchannel_side.norm()));
        if (subchannel_side(0) < 0)
          theta = 2.0 * libMesh::pi - theta;

        //        Real distance_side = subchannel_side.norm();
        //        Real distance_top = getRodPosition(subchannel_rods[2]).norm();
        //        if (distance_top > distance_side)
        //                  theta += libMesh::pi * 0.0;

        theta += _orientation_map[i];

        theta = trunc((theta + (libMesh::pi / 6.0)) / (libMesh::pi / 3.0)) * libMesh::pi / 3.0;

        if (_verbose)
        {
          if (z == 0)
          {
            _console << "Subchannel Position: " << p0 << std::endl;
            auto rods = getSubChannelRods(i);
            for (auto r : rods)
              _console << r << " ";
            _console << std::endl;
            _console << "Theta: " << theta / libMesh::pi * 180. << std::endl;
          }
        }

        // Assigning points for center channels
        for (unsigned int i = 0; i < points_per_center; i++)
        {
          auto new_point = rotatePoint(center_points[i], theta) + p0 + z0;
          if (_verbose)
          {
            if (z == 0)
              _console << i << " - " << new_point << std::endl;
          }
          mesh_base->add_point(new_point, node_id++);
          point_counter += 1;
        }
      }
    }
    else if (getSubchannelType(i) == EChannelType::EDGE)
    {
      for (auto z : _z_grid)
      {
        // Get height
        Point z0{0, 0, z};

        // Get suchannel position and assign to point
        auto loc_position = getSubchannelPosition(i);
        Point p0{loc_position[0], loc_position[1], 0};

        // Determine orientation of current subchannel
        auto subchannel_rods = getSubChannelRods(i);
        Point subchannel_side =
            getRodPosition(subchannel_rods[0]) + getRodPosition(subchannel_rods[1]);
        Point base_center_orientation = {0, 1};

        // Get rotation angle for current subchannel
        Real dot_prod = 0;
        for (unsigned int lp = 0; lp < 2; lp++)
          dot_prod += base_center_orientation(lp) * subchannel_side(lp);
        auto theta =
            std::acos(dot_prod / (base_center_orientation.norm() * subchannel_side.norm()));
        if (subchannel_side(0) > 0)
          theta = 2. * libMesh::pi - theta;
        theta = trunc((theta + (libMesh::pi / 6.0)) / (libMesh::pi / 3.0)) * libMesh::pi / 3.0;

        if (_verbose)
        {
          if (z == 0)
          {
            _console << "Subchannel Position: " << p0 << std::endl;
            auto rods = getSubChannelRods(i);
            for (auto r : rods)
              _console << r << " ";
            _console << std::endl;
            _console << "Theta: " << theta * 180 / libMesh::pi << std::endl;
          }
        }

        // Assigning points for center channels
        for (unsigned int i = 0; i < points_per_side; i++)
        {
          auto new_point = rotatePoint(side_points[i], theta) + p0 + z0;
          if (_verbose)
          {
            if (z == 0)
              _console << i << " - " << new_point << std::endl;
          }
          mesh_base->add_point(new_point, node_id++);
          point_counter += 1;
        }
      }
    }
    else // getSubchannelType(i) == EChannelType::CORNER
    {
      for (auto z : _z_grid)
      {
        // Get height
        Point z0{0, 0, z};

        // Get suchannel position and assign to point
        auto loc_position = getSubchannelPosition(i);
        Point p0{loc_position[0], loc_position[1], 0};

        // Determine orientation of current subchannel
        auto subchannel_rods = getSubChannelRods(i);
        Point subchannel_side =
            getRodPosition(subchannel_rods[0]) + getRodPosition(subchannel_rods[1]);
        Point base_center_orientation = {1, 1};

        // Get rotation angle for current subchannel
        Real dot_prod = 0;
        for (unsigned int lp = 0; lp < 2; lp++)
          dot_prod += base_center_orientation(lp) * subchannel_side(lp);
        auto theta =
            std::acos(dot_prod / (base_center_orientation.norm() * subchannel_side.norm()));
        if (subchannel_side(0) > 0)
          theta = 2. * libMesh::pi - theta;
        theta = trunc((theta + (libMesh::pi / 6.0)) / (libMesh::pi / 3.0)) * libMesh::pi / 3.0;

        if (_verbose)
        {
          if (z == 0)
          {
            _console << "Subchannel Position: " << p0 << std::endl;
            auto rods = getSubChannelRods(i);
            for (auto r : rods)
              _console << r << " ";
            _console << std::endl;
            _console << "Theta: " << theta * 180 / libMesh::pi << std::endl;
          }
        }

        // Assigning points for center channels
        for (unsigned int i = 0; i < points_per_corner; i++)
        {
          auto new_point = rotatePoint(corner_points[i], theta) + p0 + z0;
          if (_verbose)
          {
            if (z == 0)
              _console << i << " - " << new_point << std::endl;
          }
          mesh_base->add_point(new_point, node_id++);
          point_counter += 1;
        }
      }
    }
  } // i
  if (_verbose)
    _console << "Point counter: " << point_counter << std::endl;

  int element_counter = 0;
  unsigned int elem_id = 0;
  unsigned int number_of_corner = 0;
  unsigned int number_of_side = 0;
  unsigned int number_of_center = 0;
  unsigned int elems_per_channel = 0;
  unsigned int points_per_channel = 0;
  for (unsigned int i = 0; i < _n_channels; i++)
  {
    auto subch_type = getSubchannelType(i);
    if (subch_type == EChannelType::CORNER)
    {
      number_of_corner++;
      elems_per_channel = elems_per_corner;
      points_per_channel = points_per_corner;
      if (_verbose)
        _console << "Corner" << std::endl;
    }
    else if (subch_type == EChannelType::EDGE)
    {
      number_of_side++;
      elems_per_channel = elems_per_side;
      points_per_channel = points_per_side;
      if (_verbose)
        _console << "Edge" << std::endl;
    }
    else if (subch_type == EChannelType::CENTER)
    {
      number_of_center++;
      elems_per_channel = elems_per_center;
      points_per_channel = points_per_center;
      if (_verbose)
        _console << "Center" << std::endl;
    }
    for (unsigned int iz = 0; iz < _n_cells; iz++)
    {
      unsigned int elapsed_points = number_of_corner * points_per_corner * (_n_cells + 1) +
                                    number_of_side * points_per_side * (_n_cells + 1) +
                                    number_of_center * points_per_center * (_n_cells + 1) -
                                    points_per_channel * (_n_cells + 1);
      // index of the central node at base of cell
      unsigned int indx1 = iz * points_per_channel + elapsed_points;
      // index of the central node at top of cell
      unsigned int indx2 = (iz + 1) * points_per_channel + elapsed_points;

      for (unsigned int i = 0; i < elems_per_channel; i++)
      {
        Elem * elem = new Prism6;
        elem->subdomain_id() = _block_id;
        elem->set_id(elem_id++);
        elem = mesh_base->add_elem(elem);

        if (_verbose)
          _console << "Node 0: " << *mesh_base->node_ptr(indx1) << std::endl;

        elem->set_node(0) = mesh_base->node_ptr(indx1);
        elem->set_node(1) = mesh_base->node_ptr(indx1 + i + 1);
        if (i != elems_per_channel - 1)
          elem->set_node(2) = mesh_base->node_ptr(indx1 + i + 2);
        else
          elem->set_node(2) = mesh_base->node_ptr(indx1 + 1);

        elem->set_node(3) = mesh_base->node_ptr(indx2);
        elem->set_node(4) = mesh_base->node_ptr(indx2 + i + 1);
        if (i != elems_per_channel - 1)
          elem->set_node(5) = mesh_base->node_ptr(indx2 + i + 2);
        else
          elem->set_node(5) = mesh_base->node_ptr(indx2 + 1);

        if (iz == 0)
          boundary_info.add_side(elem, 0, 0);
        if (iz == _n_cells - 1)
          boundary_info.add_side(elem, 4, 1);

        element_counter += 1;
      }
    }
  }
  if (_verbose)
    _console << "Element counter: " << element_counter << std::endl;
  boundary_info.sideset_name(0) = "inlet";
  boundary_info.sideset_name(1) = "outlet";
  mesh_base->subdomain_name(_block_id) = name();
  if (_verbose)
    _console << "Mesh assembly done" << std::endl;
  mesh_base->prepare_for_use();

  return mesh_base;
}

Point
DetailedTriSubChannelMeshGenerator::rotatePoint(Point b, Real theta)
{

  // Building rotation matrix
  std::vector<std::vector<Real>> A;
  A.resize(3);
  for (std::vector<Real> a : A)
  {
    a.resize(3);
  }

  A[0] = {std::cos(theta), -std::sin(theta), 0.0};
  A[1] = {std::sin(theta), std::cos(theta), 0.0};
  A[2] = {0.0, 0.0, 1.0};

  // Rotating vector
  Point rotated_vector = Point(0.0, 0.0, 0.0);
  for (unsigned int i = 0; i < 3; i++)
  {
    for (unsigned int j = 0; j < 3; j++)
    {
      rotated_vector(i) += A[i][j] * b(j);
    }
  }

  return rotated_vector;
}

Point
DetailedTriSubChannelMeshGenerator::translatePoint(Point b, Point translation_vector)
{
  // Translating point
  Point translated_vector = Point(0.0, 0.0, 0.0);
  for (unsigned int i = 0; i < 3; i++)
  {
    translated_vector(i) = b(i) + translation_vector(i);
  }

  return translated_vector;
}
