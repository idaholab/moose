//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConcentricCircleMeshGenerator.h"
#include "CastUniquePointer.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/boundary_info.h"
#include "libmesh/utility.h"
#include "libmesh/mesh_smoother_laplace.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

registerMooseObject("MooseApp", ConcentricCircleMeshGenerator);

InputParameters
ConcentricCircleMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  MooseEnum portion(
      "full top_right top_left bottom_left bottom_right right_half left_half top_half bottom_half",
      "full");
  params.addRequiredParam<unsigned int>("num_sectors",
                                        "num_sectors % 2 = 0, num_sectors > 0"
                                        "Number of azimuthal sectors in each quadrant"
                                        "'num_sectors' must be an even number.");
  params.addRequiredParam<std::vector<Real>>("radii", "Radii of major concentric circles");
  params.addRequiredParam<std::vector<unsigned int>>(
      "rings", "Number of rings in each circle or in the enclosing square");
  params.addRequiredParam<bool>(
      "has_outer_square",
      "It determines if meshes for a outer square are added to concentric circle meshes.");
  params.addRangeCheckedParam<Real>(
      "pitch",
      0.0,
      "pitch>=0.0",
      "The enclosing square can be added to the completed concentric circle mesh."
      "Elements are quad meshes.");
  params.addParam<MooseEnum>("portion", portion, "Control of which part of mesh is created");
  params.addRequiredParam<bool>(
      "preserve_volumes", "Volume of concentric circles can be preserved using this function.");
  params.addParam<unsigned int>("smoothing_max_it", 1, "Number of Laplacian smoothing iterations");
  params.addClassDescription(
      "This ConcentricCircleMeshGenerator source code is to generate concentric "
      "circle meshes.");

  return params;
}

ConcentricCircleMeshGenerator::ConcentricCircleMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _num_sectors(getParam<unsigned int>("num_sectors")),
    _radii(getParam<std::vector<Real>>("radii")),
    _rings(getParam<std::vector<unsigned int>>("rings")),
    _has_outer_square(getParam<bool>("has_outer_square")),
    _pitch(getParam<Real>("pitch")),
    _preserve_volumes(getParam<bool>("preserve_volumes")),
    _smoothing_max_it(getParam<unsigned int>("smoothing_max_it")),
    _portion(getParam<MooseEnum>("portion"))
{
  declareMeshProperty("use_distributed_mesh", false);

  if (_num_sectors % 2 != 0)
    paramError("num_sectors", "must be an even number.");

  // radii data check
  for (unsigned i = 0; i < _radii.size() - 1; ++i)
  {
    if (_radii[i] == 0.0)
      paramError("radii", "must be positive numbers.");
    if (_radii[i] > _radii[i + 1])
      paramError("radii",
                 "must be provided in order by starting with the smallest radius providing the "
                 "following gradual radii.");
  }

  // size of 'rings' check
  if (_has_outer_square)
  {
    if (_rings.size() != _radii.size() + 1)
      paramError("rings", "The size of 'rings' must be equal to the size of 'radii' plus 1.");
  }
  else
  {
    if (_rings.size() != _radii.size())
      paramError("rings", "The size of 'rings' must be equal to the size of 'radii'.");
  }
  // pitch / 2 must be bigger than any raddi.
  if (_has_outer_square)
    for (unsigned i = 0; i < _radii.size(); ++i)
      if (_pitch / 2 < _radii[i])
        paramError("pitch", "The pitch / 2 must be larger than any radii.");
}

std::unique_ptr<MeshBase>
ConcentricCircleMeshGenerator::generate()
{
  auto mesh = buildReplicatedMesh(2);
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

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
        total_concentric_circles.push_back(_radii[j] / (_num_sectors / 2 + _rings[j]) *
                                           (i + _num_sectors / 2 + 1));
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
                      (_num_sectors + 1) * total_concentric_circles.size() +
                      Utility::pow<2>(_rings.back() + 2) + _num_sectors * (_rings.back() + 1) - 1;
  else
    num_total_nodes = Utility::pow<2>(_num_sectors / 2 + 1) +
                      (_num_sectors + 1) * total_concentric_circles.size();

  std::vector<Node *> nodes(num_total_nodes);

  unsigned node_id = 0;

  // for adding nodes for the square at the center of the circle
  Real xx = total_concentric_circles[0] / (_num_sectors / 2 + 1) * _num_sectors / 2;
  Point p1 = Point(xx, 0, 0);
  Point p2 = Point(0, xx, 0);
  Point p3 = Point(xx * std::sqrt(2.0) / 2, xx * std::sqrt(2.0) / 2, 0);
  for (unsigned i = 0; i <= _num_sectors / 2; ++i)
  {
    Real fx = i / (_num_sectors / 2.0);
    for (unsigned j = 0; j <= _num_sectors / 2; ++j)
    {
      Real fy = j / (_num_sectors / 2.0);
      Point p = p1 * fx * (1 - fy) + p2 * fy * (1 - fx) + p3 * fx * fy;
      nodes[node_id] = mesh->add_point(p, node_id);
      ++node_id;
    }
  }

  // for adding the outer layers of the square
  Real current_radius = total_concentric_circles[0];

  // for adding the outer circles of the square.
  for (unsigned layers = 0; layers < total_concentric_circles.size(); ++layers)
  {
    current_radius = total_concentric_circles[layers];
    for (unsigned num_outer_nodes = 0; num_outer_nodes <= _num_sectors; ++num_outer_nodes)
    {
      const Real x = current_radius * std::cos(num_outer_nodes * d_angle);
      const Real y = current_radius * std::sin(num_outer_nodes * d_angle);
      nodes[node_id] = mesh->add_point(Point(x, y, 0.0), node_id);
      ++node_id;
    }
  }

  // adding nodes for the enclosing square.
  // adding nodes for the left edge of the square.
  // applying the method for partitioning the line segments.

  if (_has_outer_square)
  {
    // putting nodes for the left side of the enclosing square.
    for (unsigned i = 0; i <= _num_sectors / 2; ++i)
    {
      const Real a1 = (_pitch / 2) * i / (_num_sectors / 2 + _rings.back() + 1);
      const Real b1 = _pitch / 2;

      const Real a2 = total_concentric_circles.back() * std::cos(M_PI / 2 - i * d_angle);
      const Real b2 = total_concentric_circles.back() * std::sin(M_PI / 2 - i * d_angle);

      for (unsigned j = 0; j <= _rings.back(); ++j)
      {
        Real x = ((_rings.back() + 1 - j) * a1 + j * a2) / (_rings.back() + 1);
        Real y = ((_rings.back() + 1 - j) * b1 + j * b2) / (_rings.back() + 1);
        nodes[node_id] = mesh->add_point(Point(x, y, 0.0), node_id);
        ++node_id;
      }
    }
    // putting nodes for the center part of the enclosing square.
    unsigned k = 1;
    for (unsigned i = 1; i <= _rings.back() + 1; ++i)
    {
      const Real a1 =
          (_pitch / 2) * (i + _num_sectors / 2) / (_num_sectors / 2 + _rings.back() + 1);
      const Real b1 = _pitch / 2;

      const Real a2 = _pitch / 2;
      const Real b2 = (_pitch / 2) * (_num_sectors / 2) / (_num_sectors / 2 + _rings.back() + 1);

      const Real a3 = total_concentric_circles.back() * std::cos(M_PI / 4);
      const Real b3 = total_concentric_circles.back() * std::sin(M_PI / 4);

      const Real a4 = ((_rings.back() + 1 - k) * a3 + k * a2) / (_rings.back() + 1);
      const Real b4 = ((_rings.back() + 1 - k) * b3 + k * b2) / (_rings.back() + 1);

      for (unsigned j = 0; j <= _rings.back() + 1; ++j)
      {
        Real x = ((_rings.back() + 1 - j) * a1 + j * a4) / (_rings.back() + 1);
        Real y = ((_rings.back() + 1 - j) * b1 + j * b4) / (_rings.back() + 1);
        nodes[node_id] = mesh->add_point(Point(x, y, 0.0), node_id);
        ++node_id;
      }
      ++k;
    }

    // putting nodes for the right part of the enclosing square.
    for (unsigned i = 1; i <= _num_sectors / 2; ++i)
    {
      const Real a1 = _pitch / 2;
      const Real b1 =
          (_pitch / 2) * (_num_sectors / 2 - i) / (_num_sectors / 2 + _rings.back() + 1);

      const Real a2 = total_concentric_circles.back() * std::cos(M_PI / 4 - i * d_angle);
      const Real b2 = total_concentric_circles.back() * std::sin(M_PI / 4 - i * d_angle);

      for (unsigned j = 0; j <= _rings.back(); ++j)
      {
        Real x = ((_rings.back() + 1 - j) * a1 + j * a2) / (_rings.back() + 1);
        Real y = ((_rings.back() + 1 - j) * b1 + j * b2) / (_rings.back() + 1);
        nodes[node_id] = mesh->add_point(Point(x, y, 0.0), node_id);
        ++node_id;
      }
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

  if (_has_outer_square)
    for (unsigned int i = 0; i < _rings.size() - 1; ++i)
      for (unsigned int j = 0; j < _rings[i]; ++j)
        subdomainIDs.push_back(i + 1);
  else
    for (unsigned int i = 0; i < _rings.size(); ++i)
      for (unsigned int j = 0; j < _rings[i]; ++j)
        subdomainIDs.push_back(i + 1);

  // adding elements in the square
  while (index <= limit)
  {
    // inner circle area (polygonal core)
    Elem * elem = mesh->add_elem(std::make_unique<Quad4>());
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

    // increment by 2 indices is necessary depending on where the index points to.
    if ((index - standard / 2) % (standard / 2 + 1) == 0)
      ++index;
  }

  index = (_num_sectors / 2 + 1) * (_num_sectors / 2);
  limit = (_num_sectors / 2) * (_num_sectors / 2 + 2);

  // adding elements in one outer layer of the square (right side)
  while (index < limit)
  {
    // inner circle elements touching B
    Elem * elem = mesh->add_elem(std::make_unique<Quad4>());
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
    // inner circle elements touching C
    Elem * elem = mesh->add_elem(std::make_unique<Quad4>());
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

  counter = 0;
  // adding elements for other concentric circles
  index = Utility::pow<2>(standard / 2 + 1);
  limit = Utility::pow<2>(standard / 2 + 1) +
          (_num_sectors + 1) * (total_concentric_circles.size() - 1);

  int num_nodes_boundary = Utility::pow<2>(standard / 2 + 1) + standard + 1;

  while (index < limit)
  {
    Elem * elem = mesh->add_elem(std::make_unique<Quad4>());
    elem->set_node(0) = nodes[index];
    elem->set_node(1) = nodes[index + standard + 1];
    elem->set_node(2) = nodes[index + standard + 2];
    elem->set_node(3) = nodes[index + 1];

    for (int i = 0; i < static_cast<int>(subdomainIDs.size() - 1); ++i)
      if (index < limit - (standard + 1) * i && index >= limit - (standard + 1) * (i + 1))
        elem->subdomain_id() = subdomainIDs[subdomainIDs.size() - 1 - i];

    const int initial = Utility::pow<2>(standard / 2 + 1);
    const int final = Utility::pow<2>(standard / 2 + 1) + standard - 1;

    if ((index - initial) % (standard + 1) == 0)
      boundary_info.add_side(elem, 0, 2);
    if ((index - final) % (standard + 1) == 0)
      boundary_info.add_side(elem, 2, 1);
    if (!_has_outer_square)
      if (index >= limit - (standard + 1))
        boundary_info.add_side(elem, 1, 3);

    // index increment is for adding nodes for a next element.
    ++index;

    // increment by 2 indices may be necessary depending on where the index points to.
    // this varies based on the algorithms provided for the specific element and node placement.
    if (index == (num_nodes_boundary + counter * (standard + 1)) - 1)
    {
      ++index;
      ++counter;
    }
  }

  // Enclosing square sections
  //  ABCA
  //  C  B
  //  B  C
  //  ACBA

  // adding elements for the enclosing square. (top left)
  int initial =
      Utility::pow<2>(standard / 2 + 1) + (standard + 1) * total_concentric_circles.size();

  int initial2 =
      Utility::pow<2>(standard / 2 + 1) + (standard + 1) * total_concentric_circles.size();

  if (_has_outer_square)
  {
    if (_rings.back() != 0) // this must be condition up front.
    {
      index = Utility::pow<2>(standard / 2 + 1) + (standard + 1) * total_concentric_circles.size();
      limit = Utility::pow<2>(standard / 2 + 1) + (standard + 1) * total_concentric_circles.size() +
              (_rings.back() + 1) * (standard / 2) + Utility::pow<2>(_rings.back() + 2) - 3 -
              _rings.back() * (_rings.back() + 2) - (_rings.back() + 1);
      while (index <= limit)
      {
        // outer square sector C
        Elem * elem = mesh->add_elem(std::make_unique<Quad4>());
        elem->set_node(0) = nodes[index];
        elem->set_node(1) = nodes[index + 1];
        elem->set_node(2) = nodes[index + 1 + _rings.back() + 1];
        elem->set_node(3) = nodes[index + 1 + _rings.back()];
        elem->subdomain_id() = subdomainIDs.back() + 1;

        if (index < (initial2 + static_cast<int>(_rings.back())))
          boundary_info.add_side(elem, 0, 1);

        if (index == initial)
          boundary_info.add_side(elem, 3, 4);

        ++index;

        // As mentioned before, increment by 2 indices may be required depending on where the index
        // points to.
        if ((index - initial) % static_cast<int>(_rings.back()) == 0)
        {
          ++index;
          initial = initial + (static_cast<int>(_rings.back()) + 1);
        }
      }

      // adding elements for the enclosing square. (top right)
      initial = Utility::pow<2>(standard / 2 + 1) +
                (standard + 1) * total_concentric_circles.size() +
                (_rings.back() + 1) * (standard / 2 + 1);

      limit = Utility::pow<2>(standard / 2 + 1) + (standard + 1) * total_concentric_circles.size() +
              (_rings.back() + 1) * (standard / 2) + Utility::pow<2>(_rings.back() + 2) - 3 -
              (_rings.back() + 2);

      while (index <= limit)
      {
        // outer square sector A
        Elem * elem = mesh->add_elem(std::make_unique<Quad4>());
        elem->set_node(3) = nodes[index];
        elem->set_node(2) = nodes[index + _rings.back() + 2];
        elem->set_node(1) = nodes[index + _rings.back() + 3];
        elem->set_node(0) = nodes[index + 1];
        elem->subdomain_id() = subdomainIDs.back() + 1;

        if (index >= static_cast<int>(limit - (_rings.back() + 1)))
          boundary_info.add_side(elem, 1, 3);

        if ((index - initial) % static_cast<int>(_rings.back() + 2) == 0)
          boundary_info.add_side(elem, 2, 4);

        ++index;

        if ((index - initial) % static_cast<int>(_rings.back() + 1) == 0)
        {
          ++index;
          initial = initial + (static_cast<int>(_rings.back()) + 2);
        }
      }

      // adding elements for the enclosing square. (one center quad)
      int index1 = Utility::pow<2>(standard / 2 + 1) +
                   (standard + 1) * (total_concentric_circles.size() - 1) + standard / 2;

      int index2 = Utility::pow<2>(standard / 2 + 1) +
                   (standard + 1) * total_concentric_circles.size() +
                   (_rings.back() + 1) * (standard / 2) + Utility::pow<2>(_rings.back() + 2) - 3 -
                   _rings.back() * (_rings.back() + 2) - (_rings.back() + 1);

      // pointy tips of the A sectors, touching the inner circle
      Elem * elem = mesh->add_elem(std::make_unique<Quad4>());
      elem->set_node(3) = nodes[index1];
      elem->set_node(2) = nodes[index2];
      elem->set_node(1) = nodes[index2 + _rings.back() + 1];
      elem->set_node(0) = nodes[index2 + _rings.back() + 2];
      elem->subdomain_id() = subdomainIDs.back() + 1;

      // adding elements for the left mid part.
      index = Utility::pow<2>(standard / 2 + 1) + standard / 2 +
              (standard + 1) * (total_concentric_circles.size() - 1);
      limit = index + standard / 2 - 1;

      while (index <= limit)
      {
        // outer square elements in sector C touching the inner circle
        Elem * elem = mesh->add_elem(std::make_unique<Quad4>());
        elem->set_node(3) = nodes[index];
        elem->set_node(2) = nodes[index + 1];
        elem->set_node(1) = nodes[index2 - _rings.back() - 1];
        elem->set_node(0) = nodes[index2];
        elem->subdomain_id() = subdomainIDs.back() + 1;

        if (index == limit)
          boundary_info.add_side(elem, 1, 1);

        ++index;

        // two different indices are used to add nodes for an element.
        index2 = index2 - _rings.back() - 1;
      }

      // adding elements for the right mid part.
      index1 = Utility::pow<2>(standard / 2 + 1) + standard / 2 +
               (standard + 1) * (total_concentric_circles.size() - 1);
      index2 = Utility::pow<2>(standard / 2 + 1) +
               (standard + 1) * total_concentric_circles.size() +
               (_rings.back() + 1) * (standard / 2) + Utility::pow<2>(_rings.back() + 2) - 2 +
               (_rings.back() + 1);
      int index3 =
          Utility::pow<2>(standard / 2 + 1) + (standard + 1) * total_concentric_circles.size() +
          (_rings.back() + 1) * (standard / 2) - 1 + (_rings.back() + 1) + (_rings.back() + 2);

      // elements clockwise from the A sector tips
      elem = mesh->add_elem(std::make_unique<Quad4>());
      elem->set_node(0) = nodes[index1];
      elem->set_node(1) = nodes[index1 - 1];
      elem->set_node(2) = nodes[index2];
      elem->set_node(3) = nodes[index3];
      elem->subdomain_id() = subdomainIDs.back() + 1;

      if (standard == 2)
        boundary_info.add_side(elem, 1, 2);

      // adding elements for the right mid bottom part.

      index = Utility::pow<2>(standard / 2 + 1) + standard / 2 +
              (standard + 1) * (total_concentric_circles.size() - 1) - 2;
      index1 = Utility::pow<2>(standard / 2 + 1) +
               (standard + 1) * total_concentric_circles.size() +
               (_rings.back() + 1) * (standard / 2) + Utility::pow<2>(_rings.back() + 2) - 2 +
               (_rings.back() + 1) * 2;

      limit = Utility::pow<2>(standard / 2 + 1) + standard / 2 +
              (standard + 1) * (total_concentric_circles.size() - 1) - standard / 2;

      if (standard != 2)
      {
        while (index >= limit)
        {
          // outer square elements in sector B touching the inner circle
          Elem * elem = mesh->add_elem(std::make_unique<Quad4>());
          elem->set_node(0) = nodes[index];
          elem->set_node(1) = nodes[index1];
          elem->set_node(2) = nodes[index1 - (_rings.back() + 1)];
          elem->set_node(3) = nodes[index + 1];
          elem->subdomain_id() = subdomainIDs.back() + 1;

          if (index == limit)
            boundary_info.add_side(elem, 0, 2);
          --index;
          index1 = index1 + (_rings.back() + 1);
        }
      }

      // adding elements for the right low part.
      index = Utility::pow<2>(standard / 2 + 1) + (standard + 1) * total_concentric_circles.size() +
              (_rings.back() + 1) * (standard / 2) + Utility::pow<2>(_rings.back() + 2) - 2;

      index1 = index - (_rings.back() + 2);
      // dummy condition for elem definition
      if (standard >= 2)
      {
        // single elements between A and B on the outside of the square
        Elem * elem = mesh->add_elem(std::make_unique<Quad4>());
        elem->set_node(3) = nodes[index];
        elem->set_node(2) = nodes[index + 1];
        elem->set_node(1) = nodes[index + 2];
        elem->set_node(0) = nodes[index1];
        elem->subdomain_id() = subdomainIDs.back() + 1;

        boundary_info.add_side(elem, 2, 3);

        if (standard == 2)
          boundary_info.add_side(elem, 1, 2);
      }

      index = Utility::pow<2>(standard / 2 + 1) + (standard + 1) * total_concentric_circles.size() +
              (_rings.back() + 1) * (standard / 2) + Utility::pow<2>(_rings.back() + 2) - 2 -
              (_rings.back() + 2);

      limit = Utility::pow<2>(standard / 2 + 1) + (standard + 1) * total_concentric_circles.size() +
              (_rings.back() + 1) * (standard / 2) + (_rings.back() + 2) * 2 - 2;

      int k = 1;
      while (index > limit)
      {
        Elem * elem = mesh->add_elem(std::make_unique<Quad4>());
        elem->set_node(3) = nodes[index];
        elem->set_node(2) = nodes[index + (_rings.back() + 2) * k + k + 1];
        elem->set_node(1) = nodes[index + (_rings.back() + 2) * k + k + 2];
        elem->set_node(0) = nodes[index - _rings.back() - 2];
        elem->subdomain_id() = subdomainIDs.back() + 1;
        index = index - (_rings.back() + 2);
        ++k;

        if (standard == 2)
          boundary_info.add_side(elem, 1, 2);
      }

      index = Utility::pow<2>(standard / 2 + 1) + (standard + 1) * total_concentric_circles.size() +
              (_rings.back() + 1) * (standard / 2) + Utility::pow<2>(_rings.back() + 2) - 1;
      initial = Utility::pow<2>(standard / 2 + 1) +
                (standard + 1) * total_concentric_circles.size() +
                (_rings.back() + 1) * (standard / 2) + Utility::pow<2>(_rings.back() + 2) - 1;
      limit = Utility::pow<2>(standard / 2 + 1) + (standard + 1) * total_concentric_circles.size() +
              (_rings.back() + 1) * (standard / 2) * 2 + Utility::pow<2>(_rings.back() + 2) - 2 -
              _rings.back() - 1;

      initial2 = Utility::pow<2>(standard / 2 + 1) +
                 (standard + 1) * total_concentric_circles.size() +
                 (_rings.back() + 1) * (standard / 2) * 2 + Utility::pow<2>(_rings.back() + 2) - 2 -
                 (_rings.back() + 1) * 2;

      if (standard > 2)
      {
        while (index < limit)
        {
          Elem * elem = mesh->add_elem(std::make_unique<Quad4>());
          elem->set_node(0) = nodes[index];
          elem->set_node(1) = nodes[index + 1];
          elem->set_node(2) = nodes[index + 1 + _rings.back() + 1];
          elem->set_node(3) = nodes[index + 1 + _rings.back()];
          elem->subdomain_id() = subdomainIDs.back() + 1;

          if (index > initial2)
            boundary_info.add_side(elem, 2, 2);

          if ((index - initial) == 0)
            boundary_info.add_side(elem, 3, 3);

          ++index;

          if ((index - initial) % static_cast<int>(_rings.back()) == 0)
          {
            ++index;
            initial = initial + (static_cast<int>(_rings.back()) + 1);
          }
        }
      }
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
    MeshTools::Modification::rotate(*mesh, 90, 0, 0);
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
    MeshTools::Modification::rotate(*mesh, 180, 0, 0);
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
    MeshTools::Modification::rotate(*mesh, 270, 0, 0);
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
    ReplicatedMesh other_mesh(*mesh);
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
      mesh->prepare_for_use();
      other_mesh.prepare_for_use();
      mesh->stitch_meshes(other_mesh, 1, 3, TOLERANCE, true, /*verbose=*/false);
      mesh->get_boundary_info().sideset_name(1) = "left";
      mesh->get_boundary_info().sideset_name(2) = "bottom";
      mesh->get_boundary_info().sideset_name(3) = "right";
      mesh->get_boundary_info().sideset_name(4) = "top";
    }
    else
    {
      MeshTools::Modification::change_boundary_id(other_mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(other_mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(other_mesh, 5, 2);
      mesh->prepare_for_use();
      other_mesh.prepare_for_use();
      mesh->stitch_meshes(other_mesh, 1, 1, TOLERANCE, true, /*verbose=*/false);

      MeshTools::Modification::change_boundary_id(*mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(*mesh, 3, 2);
      mesh->get_boundary_info().sideset_name(1) = "bottom";
      mesh->get_boundary_info().sideset_name(2) = "outer";
    }
    other_mesh.clear();
  }

  else if (_portion == "right_half")
  {
    ReplicatedMesh other_mesh(*mesh);
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
      mesh->prepare_for_use();
      other_mesh.prepare_for_use();
      mesh->stitch_meshes(other_mesh, 2, 4, TOLERANCE, true, /*verbose=*/false);
      mesh->get_boundary_info().sideset_name(1) = "left";
      mesh->get_boundary_info().sideset_name(2) = "bottom";
      mesh->get_boundary_info().sideset_name(3) = "right";
      mesh->get_boundary_info().sideset_name(4) = "top";
    }
    else
    {
      MeshTools::Modification::change_boundary_id(other_mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(other_mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(other_mesh, 5, 2);
      mesh->prepare_for_use();
      other_mesh.prepare_for_use();
      mesh->stitch_meshes(other_mesh, 2, 2, TOLERANCE, true, /*verbose=*/false);

      MeshTools::Modification::change_boundary_id(*mesh, 3, 2);
      mesh->get_boundary_info().sideset_name(1) = "left";
      mesh->get_boundary_info().sideset_name(2) = "outer";
    }
    other_mesh.clear();
  }
  else if (_portion == "left_half")
  {
    ReplicatedMesh other_mesh(*mesh);

    // This is to rotate the mesh and to reset boundary IDs.
    MeshTools::Modification::rotate(other_mesh, 90, 0, 0);
    MeshTools::Modification::rotate(*mesh, 180, 0, 0);
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
      MeshTools::Modification::change_boundary_id(*mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(*mesh, 2, 6);
      MeshTools::Modification::change_boundary_id(*mesh, 3, 7);
      MeshTools::Modification::change_boundary_id(*mesh, 4, 2);
      MeshTools::Modification::change_boundary_id(*mesh, 5, 3);
      MeshTools::Modification::change_boundary_id(*mesh, 6, 4);
      MeshTools::Modification::change_boundary_id(*mesh, 7, 1);
      mesh->prepare_for_use();
      other_mesh.prepare_for_use();
      mesh->stitch_meshes(other_mesh, 4, 2, TOLERANCE, true, /*verbose=*/false);
      mesh->get_boundary_info().sideset_name(1) = "left";
      mesh->get_boundary_info().sideset_name(2) = "bottom";
      mesh->get_boundary_info().sideset_name(3) = "right";
      mesh->get_boundary_info().sideset_name(4) = "top";
    }
    else
    {
      MeshTools::Modification::change_boundary_id(*mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(*mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(*mesh, 5, 2);
      mesh->prepare_for_use();
      other_mesh.prepare_for_use();
      mesh->stitch_meshes(other_mesh, 1, 1, TOLERANCE, true, /*verbose=*/false);

      MeshTools::Modification::change_boundary_id(*mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(*mesh, 3, 2);
      mesh->get_boundary_info().sideset_name(1) = "right";
      mesh->get_boundary_info().sideset_name(2) = "outer";
    }
    other_mesh.clear();
  }
  else if (_portion == "bottom_half")
  {
    ReplicatedMesh other_mesh(*mesh);
    // This is to rotate the mesh and also to reset boundary IDs.
    MeshTools::Modification::rotate(other_mesh, 180, 0, 0);
    MeshTools::Modification::rotate(*mesh, 270, 0, 0);
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
      MeshTools::Modification::change_boundary_id(*mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(*mesh, 2, 6);
      MeshTools::Modification::change_boundary_id(*mesh, 3, 7);
      MeshTools::Modification::change_boundary_id(*mesh, 4, 3);
      MeshTools::Modification::change_boundary_id(*mesh, 5, 4);
      MeshTools::Modification::change_boundary_id(*mesh, 6, 1);
      MeshTools::Modification::change_boundary_id(*mesh, 7, 2);
      mesh->prepare_for_use();
      other_mesh.prepare_for_use();
      mesh->stitch_meshes(other_mesh, 1, 3, TOLERANCE, true, /*verbose=*/false);
      mesh->get_boundary_info().sideset_name(1) = "left";
      mesh->get_boundary_info().sideset_name(2) = "bottom";
      mesh->get_boundary_info().sideset_name(3) = "right";
      mesh->get_boundary_info().sideset_name(4) = "top";
    }
    else
    {
      MeshTools::Modification::change_boundary_id(*mesh, 1, 5);
      MeshTools::Modification::change_boundary_id(*mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(*mesh, 5, 2);
      mesh->prepare_for_use();
      other_mesh.prepare_for_use();
      mesh->stitch_meshes(other_mesh, 1, 1, TOLERANCE, true, /*verbose=*/false);

      MeshTools::Modification::change_boundary_id(*mesh, 2, 1);
      MeshTools::Modification::change_boundary_id(*mesh, 3, 2);
      mesh->get_boundary_info().sideset_name(1) = "top";
      mesh->get_boundary_info().sideset_name(2) = "outer";
    }
    other_mesh.clear();
  }
  else if (_portion == "full")
  {
    ReplicatedMesh portion_two(*mesh);

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
      mesh->prepare_for_use();
      portion_two.prepare_for_use();
      // 'top_half'
      mesh->stitch_meshes(portion_two, 1, 3, TOLERANCE, true, /*verbose=*/false);

      // 'bottom_half'
      ReplicatedMesh portion_bottom(*mesh);
      MeshTools::Modification::rotate(portion_bottom, 180, 0, 0);
      MeshTools::Modification::change_boundary_id(portion_bottom, 1, 5);
      MeshTools::Modification::change_boundary_id(portion_bottom, 2, 6);
      MeshTools::Modification::change_boundary_id(portion_bottom, 3, 7);
      MeshTools::Modification::change_boundary_id(portion_bottom, 4, 2);
      MeshTools::Modification::change_boundary_id(portion_bottom, 5, 3);
      MeshTools::Modification::change_boundary_id(portion_bottom, 6, 4);
      MeshTools::Modification::change_boundary_id(portion_bottom, 7, 1);
      mesh->prepare_for_use();
      portion_bottom.prepare_for_use();
      // 'full'
      mesh->stitch_meshes(portion_bottom, 2, 4, TOLERANCE, true, /*verbose=*/false);

      mesh->get_boundary_info().sideset_name(1) = "left";
      mesh->get_boundary_info().sideset_name(2) = "bottom";
      mesh->get_boundary_info().sideset_name(3) = "right";
      mesh->get_boundary_info().sideset_name(4) = "top";
      portion_bottom.clear();
    }
    else
    {
      MeshTools::Modification::change_boundary_id(portion_two, 1, 5);
      MeshTools::Modification::change_boundary_id(portion_two, 2, 1);
      MeshTools::Modification::change_boundary_id(portion_two, 5, 2);
      // 'top half'
      mesh->prepare_for_use();
      portion_two.prepare_for_use();
      mesh->stitch_meshes(portion_two, 1, 1, TOLERANCE, true, /*verbose=*/false);
      // 'bottom half'
      ReplicatedMesh portion_bottom(*mesh);
      MeshTools::Modification::rotate(portion_bottom, 180, 0, 0);
      // 'full'
      mesh->prepare_for_use();
      portion_bottom.prepare_for_use();
      mesh->stitch_meshes(portion_bottom, 2, 2, TOLERANCE, true, /*verbose=*/false);
      MeshTools::Modification::change_boundary_id(*mesh, 3, 1);
      mesh->get_boundary_info().sideset_name(1) = "outer";
      portion_bottom.clear();
    }
    portion_two.clear();
  }

  if (_portion != "top_half" && _portion != "right_half" && _portion != "left_half" &&
      _portion != "bottom_half" && _portion != "full")
    mesh->prepare_for_use();

  // Laplace smoothing
  libMesh::LaplaceMeshSmoother lms(*mesh);
  lms.smooth(_smoothing_max_it);

  mesh->prepare_for_use();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
