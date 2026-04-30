//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMDetailedQuadAssemblyMeshGenerator.h"
#include "QuadSubChannelMesh.h"

#include "libmesh/cell_prism6.h"
#include "libmesh/unstructured_mesh.h"

#include <array>
#include <cmath>

registerMooseObject("SubChannelApp", SCMDetailedQuadAssemblyMeshGenerator);

InputParameters
SCMDetailedQuadAssemblyMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Creates a detailed mesh of subchannels and fuel pins in a square lattice arrangement");
  params.addRequiredParam<Real>("pitch", "Pitch [m]");
  params.addRequiredParam<Real>("pin_diameter", "Rod diameter [m]");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addRequiredParam<unsigned int>("nx", "Number of channels in the x direction [-]");
  params.addRequiredParam<unsigned int>("ny", "Number of channels in the y direction [-]");
  params.addRequiredParam<Real>(
      "side_gap",
      "The side gap, not to be confused with the gap between pins, this refers to the gap "
      "next to the duct or else the distance between the subchannel centroid to the duct wall."
      "distance(edge pin center, duct wall) = pitch / 2 + side_gap [m]");
  params.addParam<Real>(
      "gap",
      "The side gap, not to be confused with the gap between pins, this refers to the gap "
      "next to the duct or else the distance between the subchannel centroid to the duct wall."
      "distance(edge pin center, duct wall) = pitch / 2 + side_gap [m]");
  params.deprecateParam("gap", "side_gap", "08/06/2026");
  params.addRangeCheckedParam<unsigned int>("num_radial_parts",
                                            16,
                                            "num_radial_parts>=4",
                                            "Number of radial parts (must be at least 4).");
  params.addParam<unsigned int>("subchannel_block_id", 0, "Subchannel block id.");
  params.addParam<unsigned int>("pin_block_id", 1, "Fuel pin block id.");
  return params;
}

SCMDetailedQuadAssemblyMeshGenerator::SCMDetailedQuadAssemblyMeshGenerator(
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
    _n_channels(0),
    _side_gap(isParamValid("side_gap") ? getParam<Real>("side_gap") : getParam<Real>("gap")),
    _num_radial_parts(getParam<unsigned int>("num_radial_parts")),
    _subchannel_block_id(getParam<unsigned int>("subchannel_block_id")),
    _pin_block_id(getParam<unsigned int>("pin_block_id")),
    _elem_id(0)
{
  const Real L = _unheated_length_entry + _heated_length + _unheated_length_exit;

  if (_n_cells == 0)
    mooseError(name(), ": The number of axial cells must be greater than zero");

  if (L <= 0.0)
    mooseError(name(), ": Total bundle length must be greater than zero");

  if (_nx == 0 || _ny == 0)
    mooseError(name(), ": The number of subchannels must be greater than zero in each direction");

  if (_nx < 2 && _ny < 2)
    mooseError(name(),
               ": The number of subchannels cannot be less than 2 in both directions. "
               "Smallest assembly allowed is either 2X1 or 1X2.");

  _n_channels = _nx * _ny;

  const Real dz = L / _n_cells;
  for (unsigned int i = 0; i < _n_cells + 1; i++)
    _z_grid.push_back(dz * i);

  _subchannel_position.resize(_n_channels);
  for (unsigned int i = 0; i < _n_channels; i++)
  {
    _subchannel_position[i].reserve(3);
    for (unsigned int j = 0; j < 3; j++)
      _subchannel_position.at(i).push_back(0.0);
  }

  _subch_type.resize(_n_channels);
  for (unsigned int iy = 0; iy < _ny; iy++)
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      const unsigned int i_ch = _nx * iy + ix;
      const bool is_corner = (ix == 0 && iy == 0) || (ix == _nx - 1 && iy == 0) ||
                             (ix == 0 && iy == _ny - 1) || (ix == _nx - 1 && iy == _ny - 1);
      const bool is_edge = (ix == 0 || iy == 0 || ix == _nx - 1 || iy == _ny - 1);

      if (is_corner)
        _subch_type[i_ch] = EChannelType::CORNER;
      else if (is_edge)
        _subch_type[i_ch] = EChannelType::EDGE;
      else
        _subch_type[i_ch] = EChannelType::CENTER;

      const Real offset_x = (_nx - 1) * _pitch / 2.0;
      const Real offset_y = (_ny - 1) * _pitch / 2.0;
      _subchannel_position[i_ch][0] = _pitch * ix - offset_x;
      _subchannel_position[i_ch][1] = _pitch * iy - offset_y;
    }
}

void
SCMDetailedQuadAssemblyMeshGenerator::generatePin(std::unique_ptr<MeshBase> & mesh_base,
                                                  const Point & center)
{
  const Real dalpha = 360. / _num_radial_parts;
  const Real radius = _pin_diameter / 2.;

  std::vector<std::vector<Node *>> nodes;
  nodes.resize(_n_cells + 1);
  for (unsigned int k = 0; k < _n_cells + 1; k++)
  {
    const Real elev = _z_grid[k];
    nodes[k].push_back(mesh_base->add_point(Point(center(0), center(1), elev)));
    Real alpha = 0.;
    for (unsigned int i = 0; i < _num_radial_parts; i++, alpha += dalpha)
    {
      const Real dx = radius * std::cos(alpha * M_PI / 180.);
      const Real dy = radius * std::sin(alpha * M_PI / 180.);
      nodes[k].push_back(mesh_base->add_point(Point(center(0) + dx, center(1) + dy, elev)));
    }
  }

  for (unsigned int k = 0; k < _n_cells; k++)
    for (unsigned int i = 0; i < _num_radial_parts; i++)
    {
      Elem * elem = new Prism6;
      elem->subdomain_id() = _pin_block_id;
      elem->set_id(_elem_id++);
      mesh_base->add_elem(elem);
      const unsigned int ctr_idx = 0;
      const unsigned int idx1 = (i % _num_radial_parts) + 1;
      const unsigned int idx2 = ((i + 1) % _num_radial_parts) + 1;
      elem->set_node(0, nodes[k][ctr_idx]);
      elem->set_node(1, nodes[k][idx1]);
      elem->set_node(2, nodes[k][idx2]);
      elem->set_node(3, nodes[k + 1][ctr_idx]);
      elem->set_node(4, nodes[k + 1][idx1]);
      elem->set_node(5, nodes[k + 1][idx2]);
    }
}

std::unique_ptr<MeshBase>
SCMDetailedQuadAssemblyMeshGenerator::generate()
{
  auto mesh_base = buildMeshBaseObject();
  BoundaryInfo & boundary_info = mesh_base->get_boundary_info();
  mesh_base->set_spatial_dimension(3);

  const unsigned int n_pins = (_nx - 1) * (_ny - 1);

  const unsigned int theta_res = 16;
  const unsigned int points_per_quad = theta_res / 4 + 1;
  const unsigned int points_per_center = points_per_quad * 4 + 1;
  const unsigned int points_per_corner = points_per_quad * 1 + 1 + 3;
  const unsigned int points_per_side = points_per_quad * 2 + 1 + 2;
  const unsigned int elems_per_center = theta_res + 4;
  const unsigned int elems_per_corner = theta_res / 4 + 4;
  const unsigned int elems_per_side = theta_res / 2 + 4;

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

  const unsigned int points_per_level =
      n_corner * points_per_corner + n_side * points_per_side + n_center * points_per_center;
  const unsigned int elems_per_level =
      n_corner * elems_per_corner + n_side * elems_per_side + n_center * elems_per_center;

  std::vector<Point> pin_centers;
  if (n_pins > 0)
    QuadSubChannelMesh::generatePinCenters(_nx, _ny, _pitch, 0, pin_centers);

  const unsigned int pin_points =
      n_pins > 0 ? (_n_cells + 1) * (_num_radial_parts + 1) * pin_centers.size() : 0;
  const unsigned int pin_elems = n_pins > 0 ? _n_cells * _num_radial_parts * pin_centers.size() : 0;

  mesh_base->reserve_nodes(points_per_level * (_n_cells + 1) + pin_points);
  mesh_base->reserve_elem(elems_per_level * _n_cells + pin_elems);

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

  const Real shrink_factor = 0.99999;
  std::array<Point, 4> quadrant_centers;
  quadrant_centers[0] = Point(_pitch * 0.5 * shrink_factor, _pitch * 0.5 * shrink_factor, 0);
  quadrant_centers[1] = Point(-_pitch * 0.5 * shrink_factor, _pitch * 0.5 * shrink_factor, 0);
  quadrant_centers[2] = Point(-_pitch * 0.5 * shrink_factor, -_pitch * 0.5 * shrink_factor, 0);
  quadrant_centers[3] = Point(_pitch * 0.5 * shrink_factor, -_pitch * 0.5 * shrink_factor, 0);

  const unsigned int m = theta_res / 4;
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

  std::array<Point, points_per_corner> tl_corner_points;
  {
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[2 * m - ii];
      tl_corner_points[ii + 1] = quadrant_centers[3] + c_pt;
    }
    tl_corner_points[points_per_quad + 1] = Point(_pitch * 0.5 * shrink_factor, _side_gap, 0);
    tl_corner_points[points_per_quad + 2] = Point(-_side_gap, _side_gap, 0);
    tl_corner_points[points_per_quad + 3] = Point(-_side_gap, -_pitch * 0.5 * shrink_factor, 0);
  }

  std::array<Point, points_per_corner> tr_corner_points;
  {
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[m - ii];
      tr_corner_points[ii + 1] = quadrant_centers[2] + c_pt;
    }
    tr_corner_points[points_per_quad + 1] = Point(_side_gap, -_pitch * 0.5 * shrink_factor, 0);
    tr_corner_points[points_per_quad + 2] = Point(_side_gap, _side_gap, 0);
    tr_corner_points[points_per_quad + 3] = Point(-_pitch * 0.5 * shrink_factor, _side_gap, 0);
  }

  std::array<Point, points_per_corner> bl_corner_points;
  {
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[3 * m - ii];
      bl_corner_points[ii + 1] = quadrant_centers[0] + c_pt;
    }
    bl_corner_points[points_per_quad + 1] = Point(-_side_gap, _pitch * 0.5 * shrink_factor, 0);
    bl_corner_points[points_per_quad + 2] = Point(-_side_gap, -_side_gap, 0);
    bl_corner_points[points_per_quad + 3] = Point(_pitch * 0.5 * shrink_factor, -_side_gap, 0);
  }

  std::array<Point, points_per_corner> br_corner_points;
  {
    for (unsigned int ii = 0; ii < points_per_quad; ii++)
    {
      auto c_pt = circle_points[4 * m - ii];
      br_corner_points[ii + 1] = quadrant_centers[1] + c_pt;
    }
    br_corner_points[points_per_quad + 1] = Point(-_pitch * 0.5 * shrink_factor, -_side_gap, 0);
    br_corner_points[points_per_quad + 2] = Point(_side_gap, -_side_gap, 0);
    br_corner_points[points_per_quad + 3] = Point(_side_gap, _pitch * 0.5 * shrink_factor, 0);
  }

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
    top_points[2 * points_per_quad + 1] = Point(_pitch * 0.5 * shrink_factor, _side_gap, 0);
    top_points[2 * points_per_quad + 2] = Point(-_pitch * 0.5 * shrink_factor, _side_gap, 0);
  }

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
    left_points[2 * points_per_quad + 1] = Point(-_side_gap, _pitch * 0.5 * shrink_factor, 0);
    left_points[2 * points_per_quad + 2] = Point(-_side_gap, -_pitch * 0.5 * shrink_factor, 0);
  }

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
    bottom_points[2 * points_per_quad + 1] = Point(-_pitch * 0.5 * shrink_factor, -_side_gap, 0);
    bottom_points[2 * points_per_quad + 2] = Point(_pitch * 0.5 * shrink_factor, -_side_gap, 0);
  }

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
    right_points[2 * points_per_quad + 1] = Point(_side_gap, -_pitch * 0.5 * shrink_factor, 0);
    right_points[2 * points_per_quad + 2] = Point(_side_gap, _pitch * 0.5 * shrink_factor, 0);
  }

  if (_n_channels == 2)
  {
    unsigned int node_id = 0;
    const Real offset_x = (_nx - 1) * _pitch / 2.0;
    const Real offset_y = (_ny - 1) * _pitch / 2.0;
    for (unsigned int iy = 0; iy < _ny; iy++)
    {
      Point y0 = {0, _pitch * iy - offset_y, 0};
      for (unsigned int ix = 0; ix < _nx; ix++)
      {
        Point x0 = {_pitch * ix - offset_x, 0, 0};
        for (auto z : _z_grid)
        {
          Point z0{0, 0, z};
          if (_nx == 1 && iy == 0)
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(bottom_points[i] + x0 + y0 + z0, node_id++);
          if (_nx == 1 && iy == 1)
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(top_points[i] + x0 + y0 + z0, node_id++);
          if (_ny == 1 && ix == 0)
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(left_points[i] + x0 + y0 + z0, node_id++);
          if (_ny == 1 && ix == 1)
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(right_points[i] + x0 + y0 + z0, node_id++);
        }
      }
    }
  }
  else if (_n_channels > 2 && (_ny == 1 || _nx == 1))
  {
    unsigned int node_id = 0;
    const Real offset_x = (_nx - 1) * _pitch / 2.0;
    const Real offset_y = (_ny - 1) * _pitch / 2.0;
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
              for (unsigned int i = 0; i < points_per_side; i++)
                mesh_base->add_point(bottom_points[i] + x0 + y0 + z0, node_id++);
            else if (iy == _ny - 1)
              for (unsigned int i = 0; i < points_per_side; i++)
                mesh_base->add_point(top_points[i] + x0 + y0 + z0, node_id++);
            else
              for (unsigned int i = 0; i < points_per_center; i++)
                mesh_base->add_point(center_points[i] + x0 + y0 + z0, node_id++);
          }
          else if (_ny == 1)
          {
            if (ix == 0)
              for (unsigned int i = 0; i < points_per_side; i++)
                mesh_base->add_point(left_points[i] + x0 + y0 + z0, node_id++);
            else if (ix == _nx - 1)
              for (unsigned int i = 0; i < points_per_side; i++)
                mesh_base->add_point(right_points[i] + x0 + y0 + z0, node_id++);
            else
              for (unsigned int i = 0; i < points_per_center; i++)
                mesh_base->add_point(center_points[i] + x0 + y0 + z0, node_id++);
          }
        }
      }
    }
  }
  else
  {
    unsigned int node_id = 0;
    const Real offset_x = (_nx - 1) * _pitch / 2.0;
    const Real offset_y = (_ny - 1) * _pitch / 2.0;
    for (unsigned int iy = 0; iy < _ny; iy++)
    {
      Point y0 = {0, _pitch * iy - offset_y, 0};
      for (unsigned int ix = 0; ix < _nx; ix++)
      {
        Point x0 = {_pitch * ix - offset_x, 0, 0};
        if (ix == 0 && iy == 0)
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_corner; i++)
              mesh_base->add_point(bl_corner_points[i] + x0 + y0 + z0, node_id++);
          }
        else if (ix == _nx - 1 && iy == 0)
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_corner; i++)
              mesh_base->add_point(br_corner_points[i] + x0 + y0 + z0, node_id++);
          }
        else if (ix == 0 && iy == _ny - 1)
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_corner; i++)
              mesh_base->add_point(tl_corner_points[i] + x0 + y0 + z0, node_id++);
          }
        else if (ix == _nx - 1 && iy == _ny - 1)
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_corner; i++)
              mesh_base->add_point(tr_corner_points[i] + x0 + y0 + z0, node_id++);
          }
        else if (ix == 0 && (iy != _ny - 1 || iy != 0))
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(left_points[i] + x0 + y0 + z0, node_id++);
          }
        else if (ix == _nx - 1 && (iy != _ny - 1 || iy != 0))
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(right_points[i] + x0 + y0 + z0, node_id++);
          }
        else if (iy == 0 && (ix != _nx - 1 || ix != 0))
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(bottom_points[i] + x0 + y0 + z0, node_id++);
          }
        else if (iy == _ny - 1 && (ix != _nx - 1 || ix != 0))
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_side; i++)
              mesh_base->add_point(top_points[i] + x0 + y0 + z0, node_id++);
          }
        else
          for (auto z : _z_grid)
          {
            Point z0{0, 0, z};
            for (unsigned int i = 0; i < points_per_center; i++)
              mesh_base->add_point(center_points[i] + x0 + y0 + z0, node_id++);
          }
      }
    }
  }

  if (_n_channels == 2)
  {
    for (unsigned int iy = 0; iy < _ny; iy++)
      for (unsigned int ix = 0; ix < _nx; ix++)
      {
        const unsigned int i_ch = _nx * iy + ix;
        for (unsigned int iz = 0; iz < _n_cells; iz++)
          for (unsigned int i = 0; i < elems_per_side; i++)
          {
            Elem * elem = new Prism6;
            elem->subdomain_id() = _subchannel_block_id;
            elem->set_id(_elem_id++);
            elem = mesh_base->add_elem(elem);
            const unsigned int indx1 =
                iz * points_per_side + points_per_side * (_n_cells + 1) * i_ch;
            const unsigned int indx2 =
                (iz + 1) * points_per_side + points_per_side * (_n_cells + 1) * i_ch;
            const unsigned int elems_per_channel = elems_per_side;
            elem->set_node(0, mesh_base->node_ptr(indx1));
            elem->set_node(1, mesh_base->node_ptr(indx1 + i + 1));
            elem->set_node(2,
                           i != elems_per_channel - 1 ? mesh_base->node_ptr(indx1 + i + 2)
                                                      : mesh_base->node_ptr(indx1 + 1));
            elem->set_node(3, mesh_base->node_ptr(indx2));
            elem->set_node(4, mesh_base->node_ptr(indx2 + i + 1));
            elem->set_node(5,
                           i != elems_per_channel - 1 ? mesh_base->node_ptr(indx2 + i + 2)
                                                      : mesh_base->node_ptr(indx2 + 1));

            if (iz == 0)
              boundary_info.add_side(elem, 0, 0);
            if (iz == _n_cells - 1)
              boundary_info.add_side(elem, 4, 1);
          }
      }
  }
  else if (_n_channels > 2 && (_ny == 1 || _nx == 1))
  {
    unsigned int number_of_corner = 0;
    unsigned int number_of_side = 0;
    unsigned int number_of_center = 0;
    unsigned int elems_per_channel = 0;
    unsigned int points_per_channel = 0;
    for (unsigned int iy = 0; iy < _ny; iy++)
      for (unsigned int ix = 0; ix < _nx; ix++)
      {
        const unsigned int i_ch = _nx * iy + ix;
        const auto subch_type = getSubchannelType(i_ch);
        if (subch_type == EChannelType::CORNER)
        {
          number_of_side++;
          elems_per_channel = elems_per_side;
          points_per_channel = points_per_side;
        }
        else if (subch_type == EChannelType::EDGE)
        {
          number_of_center++;
          elems_per_channel = elems_per_center;
          points_per_channel = points_per_center;
        }

        for (unsigned int iz = 0; iz < _n_cells; iz++)
        {
          const unsigned int elapsed_points =
              number_of_corner * points_per_corner * (_n_cells + 1) +
              number_of_side * points_per_side * (_n_cells + 1) +
              number_of_center * points_per_center * (_n_cells + 1) -
              points_per_channel * (_n_cells + 1);
          const unsigned int indx1 = iz * points_per_channel + elapsed_points;
          const unsigned int indx2 = (iz + 1) * points_per_channel + elapsed_points;

          for (unsigned int i = 0; i < elems_per_channel; i++)
          {
            Elem * elem = new Prism6;
            elem->subdomain_id() = _subchannel_block_id;
            elem->set_id(_elem_id++);
            elem = mesh_base->add_elem(elem);
            elem->set_node(0, mesh_base->node_ptr(indx1));
            elem->set_node(1, mesh_base->node_ptr(indx1 + i + 1));
            elem->set_node(2,
                           i != elems_per_channel - 1 ? mesh_base->node_ptr(indx1 + i + 2)
                                                      : mesh_base->node_ptr(indx1 + 1));
            elem->set_node(3, mesh_base->node_ptr(indx2));
            elem->set_node(4, mesh_base->node_ptr(indx2 + i + 1));
            elem->set_node(5,
                           i != elems_per_channel - 1 ? mesh_base->node_ptr(indx2 + i + 2)
                                                      : mesh_base->node_ptr(indx2 + 1));

            if (iz == 0)
              boundary_info.add_side(elem, 0, 0);
            if (iz == _n_cells - 1)
              boundary_info.add_side(elem, 4, 1);
          }
        }
      }
  }
  else
  {
    unsigned int number_of_corner = 0;
    unsigned int number_of_side = 0;
    unsigned int number_of_center = 0;
    unsigned int elems_per_channel = 0;
    unsigned int points_per_channel = 0;
    for (unsigned int iy = 0; iy < _ny; iy++)
      for (unsigned int ix = 0; ix < _nx; ix++)
      {
        const unsigned int i_ch = _nx * iy + ix;
        const auto subch_type = getSubchannelType(i_ch);
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
          const unsigned int elapsed_points =
              number_of_corner * points_per_corner * (_n_cells + 1) +
              number_of_side * points_per_side * (_n_cells + 1) +
              number_of_center * points_per_center * (_n_cells + 1) -
              points_per_channel * (_n_cells + 1);
          const unsigned int indx1 = iz * points_per_channel + elapsed_points;
          const unsigned int indx2 = (iz + 1) * points_per_channel + elapsed_points;

          for (unsigned int i = 0; i < elems_per_channel; i++)
          {
            Elem * elem = new Prism6;
            elem->subdomain_id() = _subchannel_block_id;
            elem->set_id(_elem_id++);
            elem = mesh_base->add_elem(elem);
            elem->set_node(0, mesh_base->node_ptr(indx1));
            elem->set_node(1, mesh_base->node_ptr(indx1 + i + 1));
            elem->set_node(2,
                           i != elems_per_channel - 1 ? mesh_base->node_ptr(indx1 + i + 2)
                                                      : mesh_base->node_ptr(indx1 + 1));
            elem->set_node(3, mesh_base->node_ptr(indx2));
            elem->set_node(4, mesh_base->node_ptr(indx2 + i + 1));
            elem->set_node(5,
                           i != elems_per_channel - 1 ? mesh_base->node_ptr(indx2 + i + 2)
                                                      : mesh_base->node_ptr(indx2 + 1));

            if (iz == 0)
              boundary_info.add_side(elem, 0, 0);
            if (iz == _n_cells - 1)
              boundary_info.add_side(elem, 4, 1);
          }
        }
      }
  }

  if (n_pins > 0)
    for (auto & ctr : pin_centers)
      generatePin(mesh_base, ctr);

  boundary_info.sideset_name(0) = "inlet";
  boundary_info.sideset_name(1) = "outlet";
  mesh_base->subdomain_name(_subchannel_block_id) = "subchannel";
  if (n_pins > 0)
    mesh_base->subdomain_name(_pin_block_id) = "fuel_pins";
  mesh_base->prepare_for_use();

  return mesh_base;
}
