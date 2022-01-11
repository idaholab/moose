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
  std::unique_ptr<MeshBase> & _mesh;
  /// ID of the external boundary of the input mesh
  boundary_id_type _input_mesh_external_bid;

  /**
   * Decides whether a boundary of a given mesh works with the algorithm used in this class.
   * @param mesh input mesh that contains the boundary to be examined
   * @param max_node_radius a reference variable to contain the maximum radius of the nodes on the
   * boundary
   * @param origin_pt origin position of the given mesh (used for azimuthal angle calculation)
   * @param bid ID of the bounadry to be examined
   * @return whether the boundary works with the algorithm
   */
  bool isBoundaryValid(ReplicatedMesh & mesh,
                       Real & max_node_radius,
                       const Point origin_pt,
                       const boundary_id_type bid) const;

  /**
   * Decides whether a boundary of a given mesh works with the algorithm used in this class.
   * @param mesh input mesh that contains the boundary to be examined
   * @param origin_pt origin position of the given mesh (used for azimuthal angle calculation)
   * @param bid ID of the bounadry to be examined
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
