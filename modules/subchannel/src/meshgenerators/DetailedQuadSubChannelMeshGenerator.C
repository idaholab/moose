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

#include "DetailedQuadSubChannelMeshGenerator.h"
#include <array>
#include <cmath>
#include "libmesh/cell_prism6.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", DetailedQuadSubChannelMeshGenerator);

InputParameters
DetailedQuadSubChannelMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Creates a detailed mesh of subchannels in a square lattice arrangement");
  params.addRequiredParam<Real>("pitch", "Pitch [m]");
  params.addRequiredParam<Real>("pin_diameter", "Rod diameter [m]");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addRequiredParam<unsigned int>("nx", "Number of channels in the x direction [-]");
  params.addRequiredParam<unsigned int>("ny", "Number of channels in the y direction [-]");
  params.addRequiredParam<Real>("gap",
                                "(Its an added distance between a perimetric pin and the duct: "
                                "Edge Pitch W = pitch/2 - pin_diameter/2 + gap) [m]");
  params.addParam<unsigned int>("block_id", 0, "Block ID used for the mesh subdomain.");
  return params;
}

DetailedQuadSubChannelMeshGenerator::DetailedQuadSubChannelMeshGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    _unheated_length_entry(getParam<Real>("unheated_length_entry")),
    _heated_length(getParam<Real>("heated_length")),
    _unheated_length_exit(getParam<Real>("unheated_length_exit")),
    _pitch(getParam<Real>("pitch")),
    _pin_diameter(getParam<Real>("pin_diameter")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _n_channels(_nx * _ny),
    _gap(getParam<Real>("gap")),
    _block_id(getParam<unsigned int>("block_id"))
{
  Real L = _unheated_length_entry + _heated_length + _unheated_length_exit;
  Real dz = L / _n_cells;
  for (unsigned int i = 0; i < _n_cells + 1; i++)
    _z_grid.push_back(dz * i);

  _subchannel_position.resize(_n_channels);
  for (unsigned int i = 0; i < _n_channels; i++)
  {
    _subchannel_position[i].reserve(3);
    for (unsigned int j = 0; j < 3; j++)
    {
      _subchannel_position.at(i).push_back(0.0);
    }
  }

  _subch_type.resize(_n_channels);
  for (unsigned int iy = 0; iy < _ny; iy++)
  {
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      unsigned int i_ch = _nx * iy + ix;
      bool is_corner = (ix == 0 && iy == 0) || (ix == _nx - 1 && iy == 0) ||
                       (ix == 0 && iy == _ny - 1) || (ix == _nx - 1 && iy == _ny - 1);
      bool is_edge = (ix == 0 || iy == 0 || ix == _nx - 1 || iy == _ny - 1);

      if (is_corner)
        _subch_type[i_ch] = EChannelType::CORNER;
      else if (is_edge)
        _subch_type[i_ch] = EChannelType::EDGE;
      else
        _subch_type[i_ch] = EChannelType::CENTER;

      // set the subchannel positions
      Real offset_x = (_nx - 1) * _pitch / 2.0;
      Real offset_y = (_ny - 1) * _pitch / 2.0;
      _subchannel_position[i_ch][0] = _pitch * ix - offset_x;
      _subchannel_position[i_ch][1] = _pitch * iy - offset_y;
    }
  }
}

std::unique_ptr<MeshBase>
DetailedQuadSubChannelMeshGenerator::generate()
{
  auto mesh_base = buildMeshBaseObject();
  BoundaryInfo & boundary_info = mesh_base->get_boundary_info();
  mesh_base->set_spatial_dimension(3);
  // Define the resolution (the number of points used to represent a circle).
  // This must be divisible by 4.
  const unsigned int theta_res = 16; // TODO: parameterize
  // Compute the number of points needed to represent one quarter of a circle.
  const unsigned int points_per_quad = theta_res / 4 + 1;

  // Compute the points needed to represent one axial cross-flow of a subchannel.
  // For the center subchannel (sc) there is one center point plus the points from 4 intersecting
  // circles.
  const unsigned int points_per_center = points_per_quad * 4 + 1;
  // For the corner sc there is one center point plus the points from 1 intersecting circle plus 3
  // corners
  const unsigned int points_per_corner = points_per_quad * 1 + 1 + 3;
  // For the side sc there is one center point plus the points from 2 intersecting circles plus 2
  // corners
  const unsigned int points_per_side = points_per_quad * 2 + 1 + 2;

  // Compute the number of elements (Prism6) which combined base creates the sub-channel
  // cross-section
  const unsigned int elems_per_center = theta_res + 4;
  const unsigned int elems_per_corner = theta_res / 4 + 4;
  const unsigned int elems_per_side = theta_res / 2 + 4;

  // specify number and type of sub-channel
  unsigned int n_center, n_side, n_corner;
  if (_n_channels == 2)
  {
    n_corner = 0;
    n_side = _n_channels;
    n_center = _n_channels - n_side - n_corner;
  }
  else if (_n_channels > 2 && (_ny == 1 || _nx == 1))
  {
    n_corner = 0;
    n_side = 2;
    n_center = _n_channels - n_side - n_corner;
  }
  else
  {
    n_corner = 4;
    n_side = 2 * (_ny - 2) + 2 * (_nx - 2);
    n_center = _n_channels - n_side - n_corner;
  }

  // Compute the total number of points and elements.
  const unsigned int points_per_level =
      n_corner * points_per_corner + n_side * points_per_side + n_center * points_per_center;
  const unsigned int elems_per_level =
      n_corner * elems_per_corner + n_side * elems_per_side + n_center * elems_per_center;
  const unsigned int n_points = points_per_level * (_n_cells + 1);
  const unsigned int n_elems = elems_per_level * _n_cells;
  mesh_base->reserve_nodes(n_points);
  mesh_base->reserve_elem(n_elems);
  // Build an array of points arranged in a circle on the xy-plane. (last and first node overlap)
  const Real radius = _pin_diameter / 2.0;
  std::array<Point, theta_res + 1> circle_points;
  {
    Real theta = 0;
    for (unsigned int i = 0; i < theta_res + 1; i++)
    {
      circle_points[i](0) = radius * std::cos(theta);
      circle_points[i](1) = radius * std::sin(theta);
      theta += 2 * M_PI / theta_res;
    }
  }
  // Define "quadrant center" reference points.  These will be the centers of
  // the 4 circles that represent the fuel rods.  These centers are
  // offset a little bit so that in the final mesh, there is a tiny gap between
  // neighboring subchannel cells.  That allows us to easily map a solution to
  // this detailed mesh with a nearest-neighbor search.
  const Real shrink_factor = 0.99999;
  std::array<Point, 4> quadrant_centers;
  quadrant_centers[0] = Point(_pitch * 0.5 * shrink_factor, _pitch * 0.5 * shrink_factor, 0);
  quadrant_centers[1] = Point(-_pitch * 0.5 * shrink_factor, _pitch * 0.5 * shrink_factor, 0);
  quadrant_centers[2] = Point(-_pitch * 0.5 * shrink_factor, -_pitch * 0.5 * shrink_factor, 0);
  quadrant_centers[3] = Point(_pitch * 0.5 * shrink_factor, -_pitch * 0.5 * shrink_factor, 0);

  const unsigned int m = theta_res / 4;
  // Build an array of points that represent a cross section of a center subchannel
  // cell.  The points are ordered in this fashion:
  //     4   3
  // 6 5       2 1
  //       0
  // 7 8       * *
  //     9   *
  std::array<Point, points_per_center> center_points;
  {
    unsigned int start;
    for (unsigned int i = 0; i < 4; i++)
    {
      if (i == 0)
        start = 3 * m;
      if (i == 1)
        start = 4 * m;
      if (i == 2)
        start = 1 * m;
      if (i == 3)
        start = 2 * m;
      for (unsigned int ii = 0; ii < points_per_quad; ii++)
      {
        auto c_pt = circle_points[start - ii];
        center_points[i * points_per_quad + ii + 1] = quadrant_centers[i] + c_pt;
      }
    }
  }

  // Build an array of points that represent a cross section of a top left corner subchannel
  // cell. The points are ordered in this fashion:
  // 5            4
  //
  //       0
  //           2  3
  // 6       1
  std::array<Point, points_per_corner> tl_corner_points;
  {
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[2 * m - ii];
      tl_corner_points[ii + 1] = quadrant_centers[3] + c_pt;
    }
    tl_corner_points[points_per_quad + 1] =
        Point(_pitch * 0.5 * shrink_factor, _pitch * 0.25 * shrink_factor + _gap, 0);
    tl_corner_points[points_per_quad + 2] =
        Point(-_pitch * 0.25 * shrink_factor - _gap, _pitch * 0.25 * shrink_factor + _gap, 0);
    tl_corner_points[points_per_quad + 3] =
        Point(-_pitch * 0.25 * shrink_factor - _gap, -_pitch * 0.5 * shrink_factor, 0);
  }

  // Build an array of points that represent a cross section of a top right corner subchannel
  // cell.  The points are ordered in this fashion:
  // 6            5
  //
  //       0
  // 1 2
  //    3         4
  std::array<Point, points_per_corner> tr_corner_points;
  {
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[m - ii];
      tr_corner_points[ii + 1] = quadrant_centers[2] + c_pt;
    }
    tr_corner_points[points_per_quad + 1] =
        Point(_pitch * 0.25 * shrink_factor + _gap, -_pitch * 0.5 * shrink_factor, 0);
    tr_corner_points[points_per_quad + 2] =
        Point(_pitch * 0.25 * shrink_factor + _gap, _pitch * 0.25 * shrink_factor + _gap, 0);
    tr_corner_points[points_per_quad + 3] =
        Point(-_pitch * 0.5 * shrink_factor, _pitch * 0.25 * shrink_factor + _gap, 0);
  }

  // Build an array of points that represent a cross section of a bottom left corner subchannel
  // cell.  The points are ordered in this fashion:
  // 4       3
  //           2  1
  //       0
  //
  // 5            6
  std::array<Point, points_per_corner> bl_corner_points;
  {
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[3 * m - ii];
      bl_corner_points[ii + 1] = quadrant_centers[0] + c_pt;
    }
    bl_corner_points[points_per_quad + 1] =
        Point(-_pitch * 0.25 * shrink_factor - _gap, _pitch * 0.5 * shrink_factor, 0);
    bl_corner_points[points_per_quad + 2] =
        Point(-_pitch * 0.25 * shrink_factor - _gap, -_pitch * 0.25 * shrink_factor - _gap, 0);
    bl_corner_points[points_per_quad + 3] =
        Point(_pitch * 0.5 * shrink_factor, -_pitch * 0.25 * shrink_factor - _gap, 0);
  }

  // Build an array of points that represent a cross section of a bottom right corner subchannel
  // cell.  The points are ordered in this fashion:
  //    1        6
  // 3 2
  //       0
  //
  // 4           5
  std::array<Point, points_per_corner> br_corner_points;
  {
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[4 * m - ii];
      br_corner_points[ii + 1] = quadrant_centers[1] + c_pt;
    }
    br_corner_points[points_per_quad + 1] =
        Point(-_pitch * 0.5 * shrink_factor, -_pitch * 0.25 * shrink_factor - _gap, 0);
    br_corner_points[points_per_quad + 2] =
        Point(_pitch * 0.25 * shrink_factor + _gap, -_pitch * 0.25 * shrink_factor - _gap, 0);
    br_corner_points[points_per_quad + 3] =
        Point(_pitch * 0.25 * shrink_factor + _gap, _pitch * 0.5 * shrink_factor, 0);
  }

  // Build an array of points that represent a cross section of a top side subchannel
  // cell.  The points are ordered in this fashion:
  // 8            7
  //
  //       0
  // 1 2        5 6
  //    3     4
  std::array<Point, points_per_side> top_points;
  {
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[m - ii];
      top_points[ii + 1] = quadrant_centers[2] + c_pt;
    }
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[2 * m - ii];
      top_points[points_per_quad + ii + 1] = quadrant_centers[3] + c_pt;
    }
    top_points[2 * points_per_quad + 1] =
        Point(_pitch * 0.5 * shrink_factor, _pitch * 0.25 * shrink_factor + _gap, 0);
    top_points[2 * points_per_quad + 2] =
        Point(-_pitch * 0.5 * shrink_factor, _pitch * 0.25 * shrink_factor + _gap, 0);
  }

  // Build an array of points that represent a cross section of a left side subchannel
  // cell.  The points are ordered in this fashion:
  // 7        6
  //            5 4
  //      0
  //            2 3
  // 8        1
  std::array<Point, points_per_side> left_points;
  {
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[2 * m - ii];
      left_points[ii + 1] = quadrant_centers[3] + c_pt;
    }
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[3 * m - ii];
      left_points[points_per_quad + ii + 1] = quadrant_centers[0] + c_pt;
    }
    left_points[2 * points_per_quad + 1] =
        Point(-_pitch * 0.25 * shrink_factor - _gap, _pitch * 0.5 * shrink_factor, 0);
    left_points[2 * points_per_quad + 2] =
        Point(-_pitch * 0.25 * shrink_factor - _gap, -_pitch * 0.5 * shrink_factor, 0);
  }

  // Build an array of points that represent a cross section of a bottom side subchannel
  // cell.  The points are ordered in this fashion:
  //    4    3
  // 6 5       2 1
  //       0
  //
  // 7           8
  std::array<Point, points_per_side> bottom_points;
  {
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[3 * m - ii];
      bottom_points[ii + 1] = quadrant_centers[0] + c_pt;
    }
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[4 * m - ii];
      bottom_points[points_per_quad + ii + 1] = quadrant_centers[1] + c_pt;
    }
    bottom_points[2 * points_per_quad + 1] =
        Point(-_pitch * 0.5 * shrink_factor, -_pitch * 0.25 * shrink_factor - _gap, 0);
    bottom_points[2 * points_per_quad + 2] =
        Point(_pitch * 0.5 * shrink_factor, -_pitch * 0.25 * shrink_factor - _gap, 0);
  }

  // Build an array of points that represent a cross section of a right side subchannel
  // cell.  The points are ordered in this fashion:
  //    1        8
  // 3 2
  //       0
  // 4 5
  //   6         7
  std::array<Point, points_per_side> right_points;
  {
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[4 * m - ii];
      right_points[ii + 1] = quadrant_centers[1] + c_pt;
    }
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[m - ii];
      right_points[points_per_quad + ii + 1] = quadrant_centers[2] + c_pt;
    }
    right_points[2 * points_per_quad + 1] =
        Point(_pitch * 0.25 * shrink_factor + _gap, -_pitch * 0.5 * shrink_factor, 0);
    right_points[2 * points_per_quad + 2] =
        Point(_pitch * 0.25 * shrink_factor + _gap, _pitch * 0.5 * shrink_factor, 0);
  }

  // Add the points to the mesh.
  if (_n_channels == 2)
  {
    unsigned int node_id = 0;
    Real offset_x = (_nx - 1) * _pitch / 2.0;
    Real offset_y = (_ny - 1) * _pitch / 2.0;
    for (unsigned int iy = 0; iy < _ny; iy++)
    {
      Point y0 = {0, _pitch * iy - offset_y, 0};
      for (unsigned int ix = 0; ix < _nx; ix++)
      {
        Point x0 = {_pitch * ix - offset_x, 0, 0};
        for (auto z : _z_grid)
        {
          Point z0{0, 0, z};
          if (_nx == 1 && iy == 0) // vertical orientation
          {
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(bottom_points[i] + x0 + y0 + z0, node_id++);
          }
          if (_nx == 1 && iy == 1) // vertical orientation
          {
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(top_points[i] + x0 + y0 + z0, node_id++);
          }
          if (_ny == 1 && ix == 0) // horizontal orientation
          {
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(left_points[i] + x0 + y0 + z0, node_id++);
          }
          if (_ny == 1 && ix == 1) // horizontal orientation
          {
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(right_points[i] + x0 + y0 + z0, node_id++);
          }
        }
      }
    }
  }
  else if (_n_channels > 2 && (_ny == 1 || _nx == 1))
  {
    unsigned int node_id = 0;
    Real offset_x = (_nx - 1) * _pitch / 2.0;
    Real offset_y = (_ny - 1) * _pitch / 2.0;
    for (unsigned int iy = 0; iy < _ny; iy++)
    {
      Point y0 = {0, _pitch * iy - offset_y, 0};
      for (unsigned int ix = 0; ix < _nx; ix++)
      {
        Point x0 = {_pitch * ix - offset_x, 0, 0};
        for (auto z : _z_grid)
        {
          Point z0{0, 0, z};
          if (_nx == 1)
          {
            if (iy == 0)
            {
              for (unsigned int i = 0; i < points_per_side; i++)
                mesh_base->add_point(bottom_points[i] + x0 + y0 + z0, node_id++);
            }
            else if (iy == _ny - 1)
            {
              for (unsigned int i = 0; i < points_per_side; i++)
                mesh_base->add_point(top_points[i] + x0 + y0 + z0, node_id++);
            }
            else
            {
              for (unsigned int i = 0; i < points_per_center; i++)
                mesh_base->add_point(center_points[i] + x0 + y0 + z0, node_id++);
            }
          }
          else if (_ny == 1)
          {
            if (ix == 0)
            {
              for (unsigned int i = 0; i < points_per_side; i++)
                mesh_base->add_point(left_points[i] + x0 + y0 + z0, node_id++);
            }
            else if (ix == _nx - 1)
            {
              for (unsigned int i = 0; i < points_per_side; i++)
                mesh_base->add_point(right_points[i] + x0 + y0 + z0, node_id++);
            }
            else
            {
              for (unsigned int i = 0; i < points_per_center; i++)
                mesh_base->add_point(center_points[i] + x0 + y0 + z0, node_id++);
            }
          }
        }
      }
    }
  }
  else
  {
    unsigned int node_id = 0;
    Real offset_x = (_nx - 1) * _pitch / 2.0;
    Real offset_y = (_ny - 1) * _pitch / 2.0;
    for (unsigned int iy = 0; iy < _ny; iy++)
    {
      Point y0 = {0, _pitch * iy - offset_y, 0};
      for (unsigned int ix = 0; ix < _nx; ix++)
      {
        Point x0 = {_pitch * ix - offset_x, 0, 0};
        if (ix == 0 && iy == 0) // Bottom Left corner
        {
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_corner; i++)
              mesh_base->add_point(bl_corner_points[i] + x0 + y0 + z0, node_id++);
          }
        }
        else if (ix == _nx - 1 && iy == 0) // Bottom right corner
        {
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_corner; i++)
              mesh_base->add_point(br_corner_points[i] + x0 + y0 + z0, node_id++);
          }
        }
        else if (ix == 0 && iy == _ny - 1) // top Left corner
        {
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_corner; i++)
              mesh_base->add_point(tl_corner_points[i] + x0 + y0 + z0, node_id++);
          }
        }
        else if (ix == _nx - 1 && iy == _ny - 1) // top right corner
        {
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_corner; i++)
              mesh_base->add_point(tr_corner_points[i] + x0 + y0 + z0, node_id++);
          }
        }
        else if (ix == 0 && (iy != _ny - 1 || iy != 0)) // left side
        {
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(left_points[i] + x0 + y0 + z0, node_id++);
          }
        }
        else if (ix == _nx - 1 && (iy != _ny - 1 || iy != 0)) // right side
        {
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(right_points[i] + x0 + y0 + z0, node_id++);
          }
        }
        else if (iy == 0 && (ix != _nx - 1 || ix != 0)) // bottom side
        {
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(bottom_points[i] + x0 + y0 + z0, node_id++);
          }
        }
        else if (iy == _ny - 1 && (ix != _nx - 1 || ix != 0)) // top side
        {
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(top_points[i] + x0 + y0 + z0, node_id++);
          }
        }
        else // center
        {
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_center; i++)
              mesh_base->add_point(center_points[i] + x0 + y0 + z0, node_id++);
          }
        }
      }
    }
  }

  // Add elements to the mesh.  The elements are 6-node prisms.  The
  // bases of these prisms form a triangulated representation of a cross-section
  // of a center subchannel.
  if (_n_channels == 2)
  {
    unsigned int elem_id = 0;
    for (unsigned int iy = 0; iy < _ny; iy++)
    {
      for (unsigned int ix = 0; ix < _nx; ix++)
      {
        unsigned int i_ch = _nx * iy + ix;
        for (unsigned int iz = 0; iz < _n_cells; iz++)
        {
          for (unsigned int i = 0; i < elems_per_side; i++)
          {
            Elem * elem = new Prism6;
            elem->subdomain_id() = _block_id;
            elem->set_id(elem_id++);
            elem = mesh_base->add_elem(elem);
            // index of the central node at base of cell
            unsigned int indx1 = iz * points_per_side + points_per_side * (_n_cells + 1) * i_ch;
            // index of the central node at top of cell
            unsigned int indx2 =
                (iz + 1) * points_per_side + points_per_side * (_n_cells + 1) * i_ch;
            unsigned int elems_per_channel = elems_per_side;
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
          }
        }
      }
    }
    boundary_info.sideset_name(0) = "inlet";
    boundary_info.sideset_name(1) = "outlet";
    mesh_base->subdomain_name(_block_id) = name();
    mesh_base->prepare_for_use();
  }
  else if (_n_channels > 2 && (_ny == 1 || _nx == 1))
  {
    unsigned int elem_id = 0;
    unsigned int number_of_corner = 0;
    unsigned int number_of_side = 0;
    unsigned int number_of_center = 0;
    unsigned int elems_per_channel = 0;
    unsigned int points_per_channel = 0;
    for (unsigned int iy = 0; iy < _ny; iy++)
    {
      for (unsigned int ix = 0; ix < _nx; ix++)
      {
        unsigned int i_ch = _nx * iy + ix;
        auto subch_type = getSubchannelType(i_ch);
        // note that in this case i use side geometry for corner subchannel
        if (subch_type == EChannelType::CORNER)
        {
          number_of_side++;
          elems_per_channel = elems_per_side;
          points_per_channel = points_per_side;
        }
        // note that in this case i use center geometry for edge subchannel
        else if (subch_type == EChannelType::EDGE)
        {
          number_of_center++;
          elems_per_channel = elems_per_center;
          points_per_channel = points_per_center;
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
          }
        }
      }
    }
    boundary_info.sideset_name(0) = "inlet";
    boundary_info.sideset_name(1) = "outlet";
    mesh_base->subdomain_name(_block_id) = name();
    mesh_base->prepare_for_use();
  }
  else
  {
    unsigned int elem_id = 0;
    unsigned int number_of_corner = 0;
    unsigned int number_of_side = 0;
    unsigned int number_of_center = 0;
    unsigned int elems_per_channel = 0;
    unsigned int points_per_channel = 0;
    for (unsigned int iy = 0; iy < _ny; iy++)
    {
      for (unsigned int ix = 0; ix < _nx; ix++)
      {
        unsigned int i_ch = _nx * iy + ix;
        auto subch_type = getSubchannelType(i_ch);
        if (subch_type == EChannelType::CORNER)
        {
          number_of_corner++;
          elems_per_channel = elems_per_corner;
          points_per_channel = points_per_corner;
        }
        else if (subch_type == EChannelType::EDGE)
        {
          number_of_side++;
          elems_per_channel = elems_per_side;
          points_per_channel = points_per_side;
        }
        else
        {
          number_of_center++;
          elems_per_channel = elems_per_center;
          points_per_channel = points_per_center;
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
          }
        }
      }
    }
    boundary_info.sideset_name(0) = "inlet";
    boundary_info.sideset_name(1) = "outlet";
    mesh_base->subdomain_name(_block_id) = name();
    mesh_base->prepare_for_use();
  }

  return mesh_base;
}
