//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "MeshGenerator.h"
// poly2tri triangulation library
#include "poly2tri/poly2tri.h"

/**
 * This TriangulatedMeshGenerator object is designed to generate a triangulated mesh between
 * a generated outer circle boundary and a provided inner mesh. The inner mesh's outer boundary
 * is used as the triangulation inner boundary.
 */
class TriangulatedMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  TriangulatedMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Subdomain ID of created tri elements
  const SubdomainID _tri_subdomain_id;
  /// inner boundary mesh
  std::unique_ptr<MeshBase> & _inner_boundary_mesh;
  /// inner boundary id
  boundary_id_type _inner_boundary_id;
  /// outer circle boundary radius
  const Real _outer_circle_radius;
  /// Number of segments in the outer circle boundary
  const unsigned int _outer_circle_num_segments;
  /// The boundary id for the generated mesh outer boundary.
  const unsigned int _outer_boundary_id;
  /// The boundary name of the generated mesh outer boundary.
  // const std::string _outer_boundary_name;
  /// Radii of concentric Steiner circles
  const std::vector<Real> _extra_circle_radii;
  /// Number of segments in each Steiner circle
  const std::vector<unsigned int> _extra_circle_num_segments;

  /**
   * Extracts the boundary nodes from the input mesh inner boundary and sorted nodes in
   * connected-order.
   * @param mesh mesh pointer for the mesh being assembled.
   * @param inner_boundary_nodes vector of inner boundary nodes, input as empty list, populated by
   * function.
   */
  void _create_sorted_boundary_node_list(std::unique_ptr<MeshBase> & mesh,
                                         std::vector<Node *> & inner_boundary_nodes);

  /**
   * Deletes and clears a list of poly2tri Point pointers.
   * @param point_list list of poly2tri Point pointers to be deleted.
   */
  void _clearPoints(std::vector<p2t::Point *> & point_list);
};
