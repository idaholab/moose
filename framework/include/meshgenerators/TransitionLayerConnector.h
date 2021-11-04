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

/**
 * This TransitionLayerConnector object is designed to generate a transition layer to connect two
 * boundaries of two input meshes.
 */
class TransitionLayerConnector : public MeshGenerator
{
public:
  static InputParameters validParams();

  TransitionLayerConnector(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Name of the mesh which contains the first input boundary
  const MeshGeneratorName _input_name_1;
  /// Name of the mesh which contains the second input boundary
  const MeshGeneratorName _input_name_2;
  /// Name of the first input boundary for the transition layer generation
  const std::vector<BoundaryName> _boundary_1;
  /// Name of the second input boundary for the transition layer generation
  const std::vector<BoundaryName> _boundary_2;
  /// Tranlation applied to the first input mesh
  const Point _mesh_1_shift;
  /// Tranlation applied to the second input mesh
  const Point _mesh_2_shift;
  /// Number of sublayers of the mesh to be generated
  const unsigned int _num_layers;
  /// Subdomain ID to be assigned to the generated transition layer
  const subdomain_id_type _block_id;
  /// ID to be assigned to the boundary that corresponds to positions_vector_1
  const boundary_id_type _input_boundary_1_id;
  /// ID to be assigned to the boundary that corresponds to positions_vector_2
  const boundary_id_type _input_boundary_2_id;
  /// ID to be assigned to the boundary that connects the starting points of positions_vectors
  const boundary_id_type _begin_side_boundary_id;
  /// ID to be assigned to the boundary that connects the ending points of positions_vectors
  const boundary_id_type _end_side_boundary_id;
  /// A boolean parameter to determine whether QUAD4 elements are used instead of TRI3 elements
  const bool _use_quad_elements;
  /// A parameter used to set up mesh biasing of the layers
  const Real _bias_parameter;
  /// Parameter used for Gaussian blurring of local node density
  const Real _sigma;
  /// Whether the input meshes are retained in the output
  const bool _keep_inputs;
  /// The mesh which contains the first input boundary
  std::unique_ptr<MeshBase> & _input_1;
  /// The mesh which contains the second input boundary
  std::unique_ptr<MeshBase> & _input_2;
  /// ID of the external boundary of the input mesh 1
  std::vector<boundary_id_type> _input_mesh_1_external_bids;
  /// ID of the external boundary of the input mesh 2
  std::vector<boundary_id_type> _input_mesh_2_external_bids;

  /**
   * Decides whether a boundary of a given mesh works with the algorithm used in this class.
   * @param mesh input mesh that contains the boundary to be examined
   * @param max_node_radius the maximum radius of the nodes on the
   * boundary
   * @param invalid_type help distinguish different types of invalid boundaries
   * @param origin_pt origin position of the given mesh (used for azimuthal angle calculation)
   * @param bid ID of the boundary to be examined
   * @return whether the boundary works with the algorithm
   */
  bool isBoundaryValid(ReplicatedMesh & mesh,
                       Real & max_node_radius,
                       unsigned short & invalid_type,
                       std::vector<dof_id_type> & boundary_ordered_node_list,
                       const Point origin_pt,
                       const boundary_id_type bid) const;

  /**
   * Decides whether a boundary of a given mesh works is an external boundary.
   * @param mesh input mesh that contains the boundary to be examined
   * @param bid ID of the boundary to be examined
   * @return whether the boundary is the external boundary of the given mesh
   */
  bool isExternalBoundary(ReplicatedMesh & mesh, const boundary_id_type bid) const;
};
