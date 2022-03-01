//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpiralAnnularMeshGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_tri3.h"

registerMooseObject("MooseApp", SpiralAnnularMeshGenerator);

InputParameters
SpiralAnnularMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredRangeCheckedParam<Real>(
      "inner_radius", "inner_radius>0.", "The size of the inner circle.");
  params.addRequiredRangeCheckedParam<Real>("outer_radius",
                                            "outer_radius>0.",
                                            "The size of the outer circle."
                                            " Logically, it has to be greater than inner_radius");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "nodes_per_ring", "nodes_per_ring>5", "Number of nodes on each ring.");
  params.addParam<bool>(
      "use_tri6", false, "Generate mesh of TRI6 elements instead of TRI3 elements.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_rings", "num_rings>1", "The number of rings.");
  params.addParam<boundary_id_type>(
      "cylinder_bid", 1, "The boundary id to use for the cylinder (inner circle)");
  params.addParam<boundary_id_type>(
      "exterior_bid", 2, "The boundary id to use for the exterior (outer circle)");
  params.addParam<Real>("initial_delta_r",
                        "Width of the initial layer of elements around the cylinder."
                        "This number should be approximately"
                        " 2 * pi * inner_radius / nodes_per_ring to ensure that the"
                        " initial layer of elements is almost equilateral");
  params.addClassDescription(
      "Creates an annular mesh based on TRI3 or TRI6 elements on several rings.");

  return params;
}

SpiralAnnularMeshGenerator::SpiralAnnularMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _inner_radius(getParam<Real>("inner_radius")),
    _outer_radius(getParam<Real>("outer_radius")),
    _radial_bias(1.0),
    _nodes_per_ring(getParam<unsigned int>("nodes_per_ring")),
    _use_tri6(getParam<bool>("use_tri6")),
    _num_rings(getParam<unsigned int>("num_rings")),
    _cylinder_bid(getParam<boundary_id_type>("cylinder_bid")),
    _exterior_bid(getParam<boundary_id_type>("exterior_bid")),
    _initial_delta_r(2 * libMesh::pi * _inner_radius / _nodes_per_ring)
{
  declareMeshProperty("use_distributed_mesh", false);

  // catch likely user errors
  if (_outer_radius <= _inner_radius)
    mooseError("SpiralAnnularMesh: outer_radius must be greater than inner_radius");
}

std::unique_ptr<MeshBase>
SpiralAnnularMeshGenerator::generate()
{
  std::unique_ptr<ReplicatedMesh> mesh = buildReplicatedMesh(2);
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  {
    // Compute the radial bias given:
    // .) the inner radius
    // .) the outer radius
    // .) the initial_delta_r
    // .) the desired number of intervals
    // Note: the exponent n used in the formula is one less than the
    // number of rings the user requests.
    Real alpha = 1.1;
    int n = _num_rings - 1;

    // lambda used to compute the residual and Jacobian for the Newton iterations.
    // We capture parameters which don't need to change from the current scope at
    // the time this lambda is declared. The values are not updated later, so we
    // can't use this for e.g. f, df, and alpha.
    auto newton = [this, n](Real & f, Real & df, const Real & alpha)
    {
      f = (1. - std::pow(alpha, n + 1)) / (1. - alpha) -
          (_outer_radius - _inner_radius) / _initial_delta_r;
      df = (-(n + 1) * (1 - alpha) * std::pow(alpha, n) + (1. - std::pow(alpha, n + 1))) /
           (1. - alpha) / (1. - alpha);
    };

    Real f, df;
    int num_iter = 1;
    newton(f, df, alpha);

    while (std::abs(f) > 1.e-9 && num_iter <= 25)
    {
      // Compute and apply update.
      Real dx = -f / df;
      alpha += dx;
      newton(f, df, alpha);
      num_iter++;
    }

    // In case the Newton iteration fails to converge.
    if (num_iter > 25)
      mooseError("Newton iteration failed to converge (more than 25 iterations).");

    // Set radial basis to the value of alpha that we computed with Newton.
    _radial_bias = alpha;
  }

  // The number of rings specified by the user does not include the ring at
  // the surface of the cylinder itself, so we increment it by one now.
  _num_rings += 1;

  // Data structure that holds pointers to the Nodes of each ring.
  std::vector<std::vector<Node *>> ring_nodes(_num_rings);

  // Initialize radius and delta_r variables.
  Real radius = _inner_radius;
  Real delta_r = _initial_delta_r;

  // Node id counter.
  unsigned int current_node_id = 0;

  for (std::size_t r = 0; r < _num_rings; ++r)
  {
    ring_nodes[r].resize(_nodes_per_ring);

    // Add nodes starting from either theta=0 or theta=pi/nodes_per_ring
    Real theta = r % 2 == 0 ? 0 : (libMesh::pi / _nodes_per_ring);
    for (std::size_t n = 0; n < _nodes_per_ring; ++n)
    {
      ring_nodes[r][n] = mesh->add_point(Point(radius * std::cos(theta), radius * std::sin(theta)),
                                         current_node_id++);
      // Update angle
      theta += 2 * libMesh::pi / _nodes_per_ring;
    }

    // Go to next ring
    radius += delta_r;
    delta_r *= _radial_bias;
  }

  // Add elements
  for (std::size_t r = 0; r < _num_rings - 1; ++r)
  {
    // even -> odd ring
    if (r % 2 == 0)
    {
      // Inner ring (n, n*, n+1)
      // Starred indices refer to nodes on the "outer" ring of this pair.
      for (std::size_t n = 0; n < _nodes_per_ring; ++n)
      {
        // Wrap around
        unsigned int np1 = (n == _nodes_per_ring - 1) ? 0 : n + 1;
        Elem * elem = mesh->add_elem(new Tri3);
        elem->set_node(0) = ring_nodes[r][n];
        elem->set_node(1) = ring_nodes[r + 1][n];
        elem->set_node(2) = ring_nodes[r][np1];

        // Add interior faces to 'cylinder' sideset if we are on ring 0.
        if (r == 0)
          boundary_info.add_side(elem->id(), /*side=*/2, _cylinder_bid);
      }

      // Outer ring (n*, n+1*, n+1)
      for (std::size_t n = 0; n < _nodes_per_ring; ++n)
      {
        // Wrap around
        unsigned int np1 = (n == _nodes_per_ring - 1) ? 0 : n + 1;
        Elem * elem = mesh->add_elem(new Tri3);
        elem->set_node(0) = ring_nodes[r + 1][n];
        elem->set_node(1) = ring_nodes[r + 1][np1];
        elem->set_node(2) = ring_nodes[r][np1];

        // Add exterior faces to 'exterior' sideset if we're on the last ring.
        // Note: this code appears in two places since we could end on either an even or odd ring.
        if (r == _num_rings - 2)
          boundary_info.add_side(elem->id(), /*side=*/0, _exterior_bid);
      }
    }
    else
    {
      // odd -> even ring
      // Inner ring (n, n+1*, n+1)
      for (std::size_t n = 0; n < _nodes_per_ring; ++n)
      {
        // Wrap around
        unsigned int np1 = (n == _nodes_per_ring - 1) ? 0 : n + 1;
        Elem * elem = mesh->add_elem(new Tri3);
        elem->set_node(0) = ring_nodes[r][n];
        elem->set_node(1) = ring_nodes[r + 1][np1];
        elem->set_node(2) = ring_nodes[r][np1];
      }

      // Outer ring (n*, n+1*, n)
      for (std::size_t n = 0; n < _nodes_per_ring; ++n)
      {
        // Wrap around
        unsigned int np1 = (n == _nodes_per_ring - 1) ? 0 : n + 1;
        Elem * elem = mesh->add_elem(new Tri3);
        elem->set_node(0) = ring_nodes[r + 1][n];
        elem->set_node(1) = ring_nodes[r + 1][np1];
        elem->set_node(2) = ring_nodes[r][n];

        // Add exterior faces to 'exterior' sideset if we're on the last ring.
        if (r == _num_rings - 2)
          boundary_info.add_side(elem->id(), /*side=*/0, _exterior_bid);
      }
    }
  }

  // Sanity check: make sure all elements have positive area. Note: we
  // can't use elem->volume() for this, as that always returns a
  // positive area regardless of the node ordering.
  // We compute (p1-p0) \cross (p2-p0) and check that the z-component is positive.
  for (const auto & elem : mesh->element_ptr_range())
  {
    Point cp = (elem->point(1) - elem->point(0)).cross(elem->point(2) - elem->point(0));
    if (cp(2) < 0.)
      mooseError("Invalid elem found with negative area");
  }

  // Create sideset names.
  boundary_info.sideset_name(_cylinder_bid) = "cylinder";
  boundary_info.sideset_name(_exterior_bid) = "exterior";

  // Find neighbors, etc.
  mesh->prepare_for_use();

  if (_use_tri6)
  {
    mesh->all_second_order(/*full_ordered=*/true);
    std::vector<unsigned int> nos;

    // Loop over the elements, moving mid-edge nodes onto the
    // nearest radius as applicable. For each element, exactly one
    // edge should lie on the same radius, so we move only that
    // mid-edge node.
    for (const auto & elem : mesh->element_ptr_range())
    {
      // Make sure we are dealing only with triangles
      libmesh_assert(elem->n_vertices() == 3);

      // Compute vertex radii
      Real radii[3] = {elem->point(0).norm(), elem->point(1).norm(), elem->point(2).norm()};

      // Compute absolute differences between radii so we can determine which two are on the same
      // circular arc.
      Real dr[3] = {std::abs(radii[0] - radii[1]),
                    std::abs(radii[1] - radii[2]),
                    std::abs(radii[2] - radii[0])};

      // Compute index of minimum dr.
      auto index = std::distance(std::begin(dr), std::min_element(std::begin(dr), std::end(dr)));

      // Make sure that the minimum found is also (almost) zero.
      if (dr[index] > TOLERANCE)
        mooseError("Error: element had no sides with nodes on same radius.");

      // Get list of all local node ids on this side. The first
      // two entries in nos correspond to the vertices, the last
      // entry corresponds to the mid-edge node.
      nos = elem->nodes_on_side(index);

      // Compute the angles associated with nodes nos[0] and nos[1].
      Real theta0 = std::atan2(elem->point(nos[0])(1), elem->point(nos[0])(0)),
           theta1 = std::atan2(elem->point(nos[1])(1), elem->point(nos[1])(0));

      // atan2 returns values in the range (-pi, pi).  If theta0
      // and theta1 have the same sign, we can simply average them
      // to get half of the acute angle between them. On the other
      // hand, if theta0 and theta1 are of opposite sign _and_ both
      // are larger than pi/2, we need to add 2*pi when averaging,
      // otherwise we will get half of the _obtuse_ angle between
      // them, and the point will flip to the other side of the
      // circle (see below).
      Real new_theta = 0.5 * (theta0 + theta1);

      // It should not be possible for both:
      // 1.) |theta0| > pi/2, and
      // 2.) |theta1| < pi/2
      // as this would not be a well-formed element.
      if ((theta0 * theta1 < 0) && (std::abs(theta0) > 0.5 * libMesh::pi) &&
          (std::abs(theta1) > 0.5 * libMesh::pi))
        new_theta = 0.5 * (theta0 + theta1 + 2 * libMesh::pi);

      // The new radius will be the radius of point nos[0] or nos[1] (they are the same!).
      Real new_r = elem->point(nos[0]).norm();

      // Finally, move the point to its new location.
      elem->point(nos[2]) = Point(new_r * std::cos(new_theta), new_r * std::sin(new_theta), 0.);
    }
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
