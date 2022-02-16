//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "PolygonMeshGeneratorBase.h"
#include "MooseEnum.h"
#include "MooseMeshUtils.h"
#include "MeshMetaDataInterface.h"

/**
 * This PeripheralRingMeshGenerator object adds a circular peripheral region to the input mesh.
 */
class PeripheralRingMeshGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  PeripheralRingMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Name of the mesh generator to get the input mesh
  const MeshGeneratorName _input_name;
  /// Number of layers of elements of the peripheral region in radial direction
  const unsigned int _peripheral_layer_num;
  /// Bias value used to induce biasing to radial meshing in peripheral ring region
  const Real _peripheral_radial_bias;
  /// Width of the peripheral ring region to be used as inner boundary layer
  const Real _peripheral_inner_boundary_layer_width;
  /// Number of radial sectors of the inner boundary layer mesh of the peripheral ring region
  const unsigned int _peripheral_inner_boundary_layer_intervals;
  /// Growth factor used to bias radial meshing of the inner boundary layer of the peripheral ring region
  const Real _peripheral_inner_boundary_layer_bias;
  /// Width of the peripheral ring region to be used as outer boundary layer
  const Real _peripheral_outer_boundary_layer_width;
  /// Number of radial sectors of the outer boundary layer mesh of the peripheral ring region
  const unsigned int _peripheral_outer_boundary_layer_intervals;
  /// Growth factor used to bias radial meshing of the outer boundary layer of the peripheral ring region
  const Real _peripheral_outer_boundary_layer_bias;
  /// Radius of the peripheral region's outer circular boundary
  const Real _peripheral_ring_radius;
  /// Volume preserving function is optional
  const bool _preserve_volumes;
  /// Name of the external boundary of the input mesh
  const BoundaryName _input_mesh_external_boundary;
  /// Subdomain ID of the added peripheral region
  const subdomain_id_type _peripheral_ring_block_id;
  /// Subdomain name of the added peripheral region
  const SubdomainName _peripheral_ring_block_name;
  /// ID of the new external boundary
  const boundary_id_type _external_boundary_id;
  /// Name of the new external boundary
  const BoundaryName _external_boundary_name;
  /// Reference to input mesh pointer
  std::unique_ptr<MeshBase> & _input;
  /// ID of the external boundary of the input mesh
  boundary_id_type _input_mesh_external_bid;

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
                       const Point origin_pt,
                       const boundary_id_type bid) const;

  /**
   * Decides whether a boundary of a given mesh works with the algorithm used in this class.
   * @param mesh input mesh that contains the boundary to be examined
   * @param origin_pt origin position of the given mesh (used for azimuthal angle calculation)
   * @param bid ID of the boundary to be examined
   * @return whether the boundary works with the algorithm
   */
  bool
  isBoundaryValid(ReplicatedMesh & mesh, const Point origin_pt, const boundary_id_type bid) const;

  /**
   * Decides whether a boundary of a given mesh works is an external boundary.
   * @param mesh input mesh that contains the boundary to be examined
   * @param bid ID of the boundary to be examined
   * @return whether the boundary is the external boundary of the given mesh
   */
  bool isExternalBoundary(ReplicatedMesh & mesh, const boundary_id_type bid) const;
};
