//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConcentricCircleMesh.h"
#include "libmesh/face_quad4.h"
#include "MooseMesh.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/boundary_info.h"
#include "libmesh/utility.h"
// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

registerMooseObject("MooseApp", ConcentricCircleMesh);

InputParameters
ConcentricCircleMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  MooseEnum portion(
      "full top_right top_left bottom_left bottom_right right_half left_half top_half bottom_half",
      "full");
  params.addRequiredParam<unsigned int>("num_sectors",
                                        "num_sectors % 2 = 0, num_sectors > 0"
                                        "Number of azimuthal sectors in each quadrant"
                                        "'num_sectors' must be an even number.");
  params.addRequiredParam<std::vector<Real>>("radii", "Radii of major concentric circles");
  params.addRequiredParam<std::vector<unsigned int>>(
      "rings", "Number of rings in each circle or in the moderator");
  params.addRequiredParam<Real>("inner_mesh_fraction",
                                "Length of inner square / radius of the innermost circle");
  params.addRequiredParam<bool>(
      "has_outer_square",
      "It determines if meshes for a outer square are added to concentric circle meshes.");
  params.addRangeCheckedParam<Real>(
      "pitch",
      0.0,
      "pitch>=0.0",
      "The moderator can be added to complete meshes for one unit cell of fuel assembly."
      "Elements are quad meshes.");
  params.addParam<MooseEnum>("portion", portion, "Control of which part of mesh is created");
  params.addRequiredParam<bool>(
      "preserve_volumes", "Volume of concentric circles can be preserved using this function.");
  params.addClassDescription("This ConcentricCircleMesh source code is to generate concentric "
                             "circle meshes.");
  return params;
}

ConcentricCircleMesh::ConcentricCircleMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _num_sectors(getParam<unsigned int>("num_sectors")),
    _radii(getParam<std::vector<Real>>("radii")),
    _rings(getParam<std::vector<unsigned int>>("rings")),
    _inner_mesh_fraction(getParam<Real>("inner_mesh_fraction")),
    _has_outer_square(getParam<bool>("has_outer_square")),
    _pitch(getParam<Real>("pitch")),
    _preserve_volumes(getParam<bool>("preserve_volumes")),
    _portion(getParam<MooseEnum>("portion"))
{

  if (_num_sectors % 2 != 0)
    mooseError("ConcentricCircleMesh: num_sectors must be an even number.");

  // radii data check
  for (unsigned i = 0; i < _radii.size() - 1; ++i)
    if (_radii[i] > _radii[i + 1])
      mooseError("Radii must be provided in order by starting with the smallest radius and "
                 "providing the following gradual radii.");

  // condition for setting the size of inner squares.
  if (_inner_mesh_fraction > std::cos(M_PI / 4))
    mooseError("The aspect ratio can not be larger than cos(PI/4).");

  // size of 'rings' check
  if (_has_outer_square)
  {
    if (_rings.size() != _radii.size() + 1)
      mooseError("The size of 'rings' must be equal to the size of 'radii' plus 1.");
  }
  else
  {
    if (_rings.size() != _radii.size())
      mooseError("The size of 'rings' must be equal to the size of 'radii'.");
  }
  // pitch / 2 must be bigger than any raddi.
  if (_has_outer_square)
    for (unsigned i = 0; i < _radii.size(); ++i)
      if (_pitch / 2 < _radii[i])
        mooseError("The pitch / 2 must be larger than any radii.");
}

void
ConcentricCircleMesh::buildMesh()
{
  // Get the actual libMesh mesh
  ReplicatedMesh & mesh = cast_ref<ReplicatedMesh &>(getMesh());
  // Set dimension of mesh
  mesh.set_mesh_dimension(2);
  mesh.set_spatial_dimension(2);
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Creating real mesh concentric circles
  // i: index for _rings, j: index for _radii
  std::vector<Real> total_concentric_circles;
  unsigned int j = 0;
  while (j < _radii.size())
  {
    unsigned int i = 0;
    if (j == 0)
      while (i < _rings[j])
      {
        total_concentric_circles.push_back(_inner_mesh_fraction * _radii[j] +
                                           (_radii[j] - _inner_mesh_fraction * _radii[j]) /
                                               _rings[j] * (i + 1));
        ++i;
      }
    else
      while (i < _rings[j])
      {
        total_concentric_circles.push_back(_radii[j - 1] +
                                           (_radii[j] - _radii[j - 1]) / _rings[j] * (i + 1));
        ++i;
      }
    ++j;
  }

  // volume preserving function is used to conserve volume.
  const Real d_angle = M_PI / 2 / _num_sectors;

  if (_preserve_volumes)
  {
    Real original_radius = 0.0;
    for (unsigned i = 0; i < total_concentric_circles.size(); ++i)
    {
      // volume preserving function for the center circle
      if (i == 0)
      {
        const Real target_area = M_PI * Utility::pow<2>(total_concentric_circles[i]);
        Real modified_radius = std::sqrt(2 * target_area / std::sin(d_angle) / _num_sectors / 4);
        original_radius = total_concentric_circles[i];
        total_concentric_circles[i] = modified_radius;
      }
      else
      {
        // volume preserving functions for outer circles
        const Real target_area = M_PI * (Utility::pow<2>(total_concentric_circles[i]) -
                                         Utility::pow<2>(original_radius));
        Real modified_radius = std::sqrt(target_area / std::sin(d_angle) / _num_sectors / 2 +
                                         Utility::pow<2>(total_concentric_circles[i - 1]));
        original_radius = total_concentric_circles[i];
        total_concentric_circles[i] = modified_radius;
      }
    }
  }

  // number of total nodes
  unsigned num_total_nodes = 0;
  if (_has_outer_square)
    num_total_nodes = Utility::pow<2>(_num_sectors / 2 + 1) +
                      (_num_sectors + 1) * (total_concentric_circles.size() + _rings.back()) +
                      (_num_sectors + 1);
  else
    num_total_nodes = Utility::pow<2>(_num_sectors / 2 + 1) +
                      (_num_sectors + 1) * total_concentric_circles.size();

  std::vector<Node *> nodes(num_total_nodes);
  unsigned node_id = 0;

  // for adding nodes for the square at the center of the circle
  for (unsigned i = 0; i <= _num_sectors / 2; ++i)
  {
    const Real x = i * _inner_mesh_fraction * total_concentric_circles[0] / (_num_sectors / 2);
    for (unsigned j = 0; j <= _num_sectors / 2; ++j)
    {
      const Real y = j * _inner_mesh_fraction * total_concentric_circles[0] / (_num_sectors / 2);
      nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
      ++node_id;
    }
  }

  // for adding the outer nodes of the square
  Real current_radius = 0.0;

  for (unsigned layers = 0; layers < total_concentric_circles.size(); ++layers)
  {
    current_radius = total_concentric_circles[layers];
    for (unsigned num_outer_nodes = 0; num_outer_nodes <= _num_sectors; ++num_outer_nodes)
    {
      const Real x = current_radius * std::cos(num_outer_nodes * d_angle);
      const Real y = current_radius * std::sin(num_outer_nodes * d_angle);
      nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
      ++node_id;
    }
  }

  // adding nodes for the unit cell of fuel assembly.
  if (_has_outer_square)
  {
    Real current_radius_moderator = 0.0;
    for (unsigned i = 1; i <= _rings.back(); ++i)
    {
      current_radius_moderator =
          _radii.back() + i * (_pitch / 2 - _radii.back()) / (_rings.back() + 1);
      total_concentric_circles.push_back(current_radius_moderator);
      for (unsigned num_outer_nodes = 0; num_outer_nodes <= _num_sectors; ++num_outer_nodes)
      {
        const Real x = current_radius_moderator * std::cos(num_outer_nodes * d_angle);
        const Real y = current_radius_moderator * std::sin(num_outer_nodes * d_angle);
        nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
        ++node_id;
      }
    }

    for (unsigned j = 0; j < _num_sectors / 2 + 1; ++j)
    {
      const Real x = _pitch / 2;
      const Real y = _pitch / 2 * std::tan(j * d_angle);
      nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
      ++node_id;
    }

    for (unsigned i = 0; i < _num_sectors / 2; ++i)
    {
      const Real x = _pitch / 2 * std::cos((i + _num_sectors / 2 + 1) * d_angle) /
                     std::sin((i + _num_sectors / 2 + 1) * d_angle);
      const Real y = _pitch / 2;
      nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
      ++node_id;
    }
  }
  // Currently, index, limit, counter variables use the int type because of the 'modulo' function.
  // adding elements
  int index = 0;
  int limit = 0;
  int standard = static_cast<int>(_num_sectors);

  // This is to set the limit for the index
  if (standard > 4)
  {
    int additional_term = 0;
    int counter = standard;
    while (counter > 4)
    {
      counter = counter - 2;
      additional_term = additional_term + counter;
    }
    limit = standard + additional_term;
  }
  else if (standard == 4)
    limit = standard;

  // SubdomainIDs set up
  std::vector<unsigned int> subdomainIDs;
  for (unsigned int i = 0; i < _rings.size(); ++i)
    for (unsigned int j = 0; j < _rings[i]; ++j)
      subdomainIDs.push_back(i + 1);

  if (_has_outer_square)
    subdomainIDs.push_back(subdomainIDs.back());
  // adding elements in the square
  while (index <= limit)
  {
    Elem * elem = mesh.add_elem(new Quad4);
    elem->set_node(0) = nodes[index];
    elem->set_node(1) = nodes[index + _num_sectors / 2 + 1];
    elem->set_node(2) = nodes[index + _num_sectors / 2 + 2];
    elem->set_node(3) = nodes[index + 1];
    elem->subdomain_id() = subdomainIDs[0];

    if (index < standard / 2)
      boundary_info.add_side(elem, 3, 1);
    if (index % (standard / 2 + 1) == 0)
      boundary_info.add_side(elem, 0, 2);

    ++index;
    if ((index - standard / 2) % (standard / 2 + 1) == 0)
      ++index;
  }

  index = (_num_sectors / 2 + 1) * (_num_sectors / 2);
  limit = (_num_sectors / 2) * (_num_sectors / 2 + 2);

  // adding elements in one outer layer of the square (right side)
  while (index < limit)
  {
    Elem * elem = mesh.add_elem(new Quad4);
    elem->set_node(0) = nodes[index];
    elem->set_node(1) = nodes[index + _num_sectors / 2 + 1];
    elem->set_node(2) = nodes[index + _num_sectors / 2 + 2];
    elem->set_node(3) = nodes[index + 1];
    elem->subdomain_id() = subdomainIDs[0];

    if (index == (standard / 2 + 1) * (standard / 2))
      boundary_info.add_side(elem, 0, 2);

    ++index;
  }

  // adding elements in one outer layer of the square (left side)
  int counter = 0;
  while (index != standard / 2)
  {
    Elem * elem = mesh.add_elem(new Quad4);
    elem->set_node(0) = nodes[index];
    elem->set_node(1) = nodes[index + (_num_sectors / 2 + 1) + counter * (_num_sectors / 2 + 2)];
    elem->set_node(2) =
        nodes[index + (_num_sectors / 2 + 1) + counter * (_num_sectors / 2 + 2) + 1];
    elem->set_node(3) = nodes[index - _num_sectors / 2 - 1];
    elem->subdomain_id() = subdomainIDs[0];

    if (index == standard + 1)
      boundary_info.add_side(elem, 2, 1);

    index = index - _num_sectors / 2 - 1;
    ++counter;
  }

  // adding elements for other concentric circles
  index = Utility::pow<2>(_num_sectors / 2 + 1);
  limit = static_cast<int>(num_total_nodes) - standard - 2;
  int num_nodes_boundary = Utility::pow<2>(_num_sectors / 2 + 1) + _num_sectors + 1;

  counter = 0;
  while (index < limit)
  {

    Elem * elem = mesh.add_elem(new Quad4);
    elem->set_node(0) = nodes[index];
    elem->set_node(1) = nodes[index + _num_sectors + 1];
    elem->set_node(2) = nodes[index + _num_sectors + 2];
    elem->set_node(3) = nodes[index + 1];

    for (int i = 0; i < static_cast<int>(subdomainIDs.size()) - 1; ++i)
      if (index < limit - (standard + 1) * i && index >= limit - (standard + 1) * (i + 1))
        elem->subdomain_id() = subdomainIDs[subdomainIDs.size() - 1 - i];

    int const initial = Utility::pow<2>(standard / 2 + 1);
    int const final = Utility::pow<2>(standard / 2 + 1) + _num_sectors - 1;

    if ((index - initial) % (standard + 1) == 0)
      boundary_info.add_side(elem, 0, 2);
    if ((index - final) % (standard + 1) == 0)
      boundary_info.add_side(elem, 2, 1);
    if (index > limit - (standard + 1))
    {
      if (_has_outer_square)
      {
        if (index < limit - standard + standard / 2)
          boundary_info.add_side(elem, 1, 3);
        else
          boundary_info.add_side(elem, 1, 4);
      }
      else
      {
        boundary_info.add_side(elem, 1, 3);
      }
    }
    ++index;
    if (index == (num_nodes_boundary + counter * (standard + 1)) - 1)
    {
      ++index;
      ++counter;
    }
  }

  // This is to set boundary names.
  boundary_info.sideset_name(1) = "left";
  boundary_info.sideset_name(2) = "bottom";

  if (!_has_outer_square)
    boundary_info.sideset_name(3) = "outer";
  else
  {
    boundary_info.sideset_name(3) = "right";
    boundary_info.sideset_name(4) = "top";
  }

  if (_portion == "top_left")
  {
    MeshTools::Modification::rotate(mesh, 90, 0, 0);
    boundary_info.sideset_name(1) = "bottom";
    boundary_info.sideset_name(2) = "right";

    if (!_has_outer_square)
      boundary_info.sideset_name(3) = "outer";
    else
    {
      boundary_info.sideset_name(3) = "top";
      boundary_info.sideset_name(4) = "left";
    }
  }
  else if (_portion == "bottom_left")
  {
    MeshTools::Modification::rotate(mesh, 180, 0, 0);
    boundary_info.sideset_name(1) = "right";
    boundary_info.sideset_name(2) = "top";

    if (!_has_outer_square)
      boundary_info.sideset_name(3) = "outer";
    else
    {
      boundary_info.sideset_name(3) = "left";
      boundary_info.sideset_name(4) = "bottom";
    }
  }
  else if (_portion == "bottom_right")
  {
    MeshTools::Modification::rotate(mesh, 270, 0, 0);
    boundary_info.sideset_name(1) = "top";
    boundary_info.sideset_name(2) = "left";

    if (!_has_outer_square)
      boundary_info.sideset_name(3) = "outer";
    else
    {
      boundary_info.sideset_name(3) = "bottom";
      boundary_info.sideset_name(4) = "right";
    }
  }

  else if (_portion == "top_half")
  {
    ReplicatedMesh other_mesh(mesh);
    // This is to rotate the mesh and also to reset boundary IDs.
    MeshTools::Modification::rotate(other_mesh, 90, 0, 0);
    if (_has_outer_square)
    {
      MeshTools::Modification::change_boundary_id(other_mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(other_mesh, 2, 6);
      MeshTools::Modification::change_boundary_id(other_mesh, 3, 7);
      MeshTools::Modification::change_boundary_id(other_mesh, 4, 1);
      MeshTools::Modification::change_boundary_id(other_mesh, 5, 2);
      MeshTools::Modification::change_boundary_id(other_mesh, 6, 3);
      MeshTools::Modification::change_boundary_id(other_mesh, 7, 4);
      mesh.prepare_for_use();
      other_mesh.prepare_for_use();
      mesh.stitch_meshes(other_mesh, 1, 3, TOLERANCE, true);
      mesh.get_boundary_info().sideset_name(1) = "left";
      mesh.get_boundary_info().sideset_name(2) = "bottom";
      mesh.get_boundary_info().sideset_name(3) = "right";
      mesh.get_boundary_info().sideset_name(4) = "top";
    }
    else
    {
      MeshTools::Modification::change_boundary_id(other_mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(other_mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(other_mesh, 5, 2);
      mesh.prepare_for_use();
      other_mesh.prepare_for_use();
      mesh.stitch_meshes(other_mesh, 1, 1, TOLERANCE, true);

      MeshTools::Modification::change_boundary_id(mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(mesh, 3, 2);
      mesh.get_boundary_info().sideset_name(1) = "bottom";
      mesh.get_boundary_info().sideset_name(2) = "outer";
    }
    other_mesh.clear();
  }

  else if (_portion == "right_half")
  {
    ReplicatedMesh other_mesh(mesh);
    // This is to rotate the mesh and also to reset boundary IDs.
    MeshTools::Modification::rotate(other_mesh, 270, 0, 0);
    if (_has_outer_square)
    {
      MeshTools::Modification::change_boundary_id(other_mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(other_mesh, 2, 6);
      MeshTools::Modification::change_boundary_id(other_mesh, 3, 7);
      MeshTools::Modification::change_boundary_id(other_mesh, 4, 3);
      MeshTools::Modification::change_boundary_id(other_mesh, 5, 4);
      MeshTools::Modification::change_boundary_id(other_mesh, 6, 1);
      MeshTools::Modification::change_boundary_id(other_mesh, 7, 2);
      mesh.prepare_for_use();
      other_mesh.prepare_for_use();
      mesh.stitch_meshes(other_mesh, 2, 4, TOLERANCE, true);
      mesh.get_boundary_info().sideset_name(1) = "left";
      mesh.get_boundary_info().sideset_name(2) = "bottom";
      mesh.get_boundary_info().sideset_name(3) = "right";
      mesh.get_boundary_info().sideset_name(4) = "top";
    }
    else
    {
      MeshTools::Modification::change_boundary_id(other_mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(other_mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(other_mesh, 5, 2);
      mesh.prepare_for_use();
      other_mesh.prepare_for_use();
      mesh.stitch_meshes(other_mesh, 2, 2, TOLERANCE, true);

      MeshTools::Modification::change_boundary_id(mesh, 3, 2);
      mesh.get_boundary_info().sideset_name(1) = "left";
      mesh.get_boundary_info().sideset_name(2) = "outer";
    }
    other_mesh.clear();
  }
  else if (_portion == "left_half")
  {
    ReplicatedMesh other_mesh(mesh);

    // This is to rotate the mesh and to reset boundary IDs.
    MeshTools::Modification::rotate(other_mesh, 90, 0, 0);
    MeshTools::Modification::rotate(mesh, 180, 0, 0);
    if (_has_outer_square)
    {
      // The other mesh is created by rotating the original mesh about 90 degrees.
      MeshTools::Modification::change_boundary_id(other_mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(other_mesh, 2, 6);
      MeshTools::Modification::change_boundary_id(other_mesh, 3, 7);
      MeshTools::Modification::change_boundary_id(other_mesh, 4, 1);
      MeshTools::Modification::change_boundary_id(other_mesh, 5, 2);
      MeshTools::Modification::change_boundary_id(other_mesh, 6, 3);
      MeshTools::Modification::change_boundary_id(other_mesh, 7, 4);
      // The original mesh is then rotated about 180 degrees.
      MeshTools::Modification::change_boundary_id(mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(mesh, 2, 6);
      MeshTools::Modification::change_boundary_id(mesh, 3, 7);
      MeshTools::Modification::change_boundary_id(mesh, 4, 2);
      MeshTools::Modification::change_boundary_id(mesh, 5, 3);
      MeshTools::Modification::change_boundary_id(mesh, 6, 4);
      MeshTools::Modification::change_boundary_id(mesh, 7, 1);
      mesh.prepare_for_use();
      other_mesh.prepare_for_use();
      mesh.stitch_meshes(other_mesh, 4, 2, TOLERANCE, true);
      mesh.get_boundary_info().sideset_name(1) = "left";
      mesh.get_boundary_info().sideset_name(2) = "bottom";
      mesh.get_boundary_info().sideset_name(3) = "right";
      mesh.get_boundary_info().sideset_name(4) = "top";
    }
    else
    {
      MeshTools::Modification::change_boundary_id(mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(mesh, 5, 2);
      mesh.prepare_for_use();
      other_mesh.prepare_for_use();
      mesh.stitch_meshes(other_mesh, 1, 1, TOLERANCE, true);

      MeshTools::Modification::change_boundary_id(mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(mesh, 3, 2);
      mesh.get_boundary_info().sideset_name(1) = "right";
      mesh.get_boundary_info().sideset_name(2) = "outer";
    }
    other_mesh.clear();
  }
  else if (_portion == "bottom_half")
  {
    ReplicatedMesh other_mesh(mesh);
    // This is to rotate the mesh and also to reset boundary IDs.
    MeshTools::Modification::rotate(other_mesh, 180, 0, 0);
    MeshTools::Modification::rotate(mesh, 270, 0, 0);
    if (_has_outer_square)
    {
      // The other mesh is created by rotating the original mesh about 180 degrees.
      MeshTools::Modification::change_boundary_id(other_mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(other_mesh, 2, 6);
      MeshTools::Modification::change_boundary_id(other_mesh, 3, 7);
      MeshTools::Modification::change_boundary_id(other_mesh, 4, 2);
      MeshTools::Modification::change_boundary_id(other_mesh, 5, 3);
      MeshTools::Modification::change_boundary_id(other_mesh, 6, 4);
      MeshTools::Modification::change_boundary_id(other_mesh, 7, 1);
      // The original mesh is rotated about 270 degrees.
      MeshTools::Modification::change_boundary_id(mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(mesh, 2, 6);
      MeshTools::Modification::change_boundary_id(mesh, 3, 7);
      MeshTools::Modification::change_boundary_id(mesh, 4, 3);
      MeshTools::Modification::change_boundary_id(mesh, 5, 4);
      MeshTools::Modification::change_boundary_id(mesh, 6, 1);
      MeshTools::Modification::change_boundary_id(mesh, 7, 2);
      mesh.prepare_for_use();
      other_mesh.prepare_for_use();
      mesh.stitch_meshes(other_mesh, 1, 3, TOLERANCE, true);
      mesh.get_boundary_info().sideset_name(1) = "left";
      mesh.get_boundary_info().sideset_name(2) = "bottom";
      mesh.get_boundary_info().sideset_name(3) = "right";
      mesh.get_boundary_info().sideset_name(4) = "top";
    }
    else
    {
      MeshTools::Modification::change_boundary_id(mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(mesh, 5, 2);
      mesh.prepare_for_use();
      other_mesh.prepare_for_use();
      mesh.stitch_meshes(other_mesh, 1, 1, TOLERANCE, true);

      MeshTools::Modification::change_boundary_id(mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(mesh, 3, 2);
      mesh.get_boundary_info().sideset_name(1) = "top";
      mesh.get_boundary_info().sideset_name(2) = "outer";
    }
    other_mesh.clear();
  }
  else if (_portion == "full")
  {
    ReplicatedMesh portion_two(mesh);

    // This is to rotate the mesh and also to reset boundary IDs.
    MeshTools::Modification::rotate(portion_two, 90, 0, 0);

    if (_has_outer_square)
    {
      // Portion 2: 2nd quadrant
      MeshTools::Modification::change_boundary_id(portion_two, 1, 5);
      MeshTools::Modification::change_boundary_id(portion_two, 2, 6);
      MeshTools::Modification::change_boundary_id(portion_two, 3, 7);
      MeshTools::Modification::change_boundary_id(portion_two, 4, 1);
      MeshTools::Modification::change_boundary_id(portion_two, 5, 2);
      MeshTools::Modification::change_boundary_id(portion_two, 6, 3);
      MeshTools::Modification::change_boundary_id(portion_two, 7, 4);
      mesh.prepare_for_use();
      portion_two.prepare_for_use();
      // 'top_half'
      mesh.stitch_meshes(portion_two, 1, 3, TOLERANCE, true);

      // 'bottom_half'
      ReplicatedMesh portion_bottom(mesh);
      MeshTools::Modification::rotate(portion_bottom, 180, 0, 0);
      MeshTools::Modification::change_boundary_id(portion_bottom, 1, 5);
      MeshTools::Modification::change_boundary_id(portion_bottom, 2, 6);
      MeshTools::Modification::change_boundary_id(portion_bottom, 3, 7);
      MeshTools::Modification::change_boundary_id(portion_bottom, 4, 2);
      MeshTools::Modification::change_boundary_id(portion_bottom, 5, 3);
      MeshTools::Modification::change_boundary_id(portion_bottom, 6, 4);
      MeshTools::Modification::change_boundary_id(portion_bottom, 7, 1);
      mesh.prepare_for_use();
      portion_bottom.prepare_for_use();
      // 'full'
      mesh.stitch_meshes(portion_bottom, 2, 4, TOLERANCE, true);

      mesh.get_boundary_info().sideset_name(1) = "left";
      mesh.get_boundary_info().sideset_name(2) = "bottom";
      mesh.get_boundary_info().sideset_name(3) = "right";
      mesh.get_boundary_info().sideset_name(4) = "top";
      portion_bottom.clear();
    }
    else
    {
      MeshTools::Modification::change_boundary_id(portion_two, 1, 5);
      MeshTools::Modification::change_boundary_id(portion_two, 2, 1);
      MeshTools::Modification::change_boundary_id(portion_two, 5, 2);
      // 'top half'
      mesh.prepare_for_use();
      portion_two.prepare_for_use();
      mesh.stitch_meshes(portion_two, 1, 1, TOLERANCE, true);
      // 'bottom half'
      ReplicatedMesh portion_bottom(mesh);
      MeshTools::Modification::rotate(portion_bottom, 180, 0, 0);
      // 'full'
      mesh.prepare_for_use();
      portion_bottom.prepare_for_use();
      mesh.stitch_meshes(portion_bottom, 2, 2, TOLERANCE, true);
      MeshTools::Modification::change_boundary_id(mesh, 3, 1);
      mesh.get_boundary_info().sideset_name(1) = "outer";
      portion_bottom.clear();
    }
    portion_two.clear();
  }
  if (_portion != "top_half" && _portion != "right_half" && _portion != "left_half" &&
      _portion != "bottom_half" && _portion != "full")
    mesh.prepare_for_use();
}
