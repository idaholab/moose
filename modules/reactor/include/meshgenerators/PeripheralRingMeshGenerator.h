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
  /// Whether to enforce use of the centroid position of the input mesh as the center of the peripheral ring
  const bool _force_input_centroid_as_center;
  /// Number of layers of elements of the peripheral region in radial direction
  const unsigned int _peripheral_layer_num;
  /// Bias value used to induce biasing to radial meshing in peripheral ring region
  const Real _peripheral_radial_bias;
  /// Width, fraction, radiation sectors and growth factor of the inner boundary layer of the peripheral region
  singleBdryLayerParams _peripheral_inner_boundary_layer_params;
  /// Width, fraction, radiation sectors and growth factor of the outer boundary layer of the peripheral region
  singleBdryLayerParams _peripheral_outer_boundary_layer_params;
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
   * Define node positions of the inner boundary layer that is conformal to the input mesh's
   * external boundary.
   * @param input_ext_node_num number of nodes on the external boundary of the input mesh
   * @param input_bdry_angles list of angles (in rad) formed by three neighboring nodes on the
   * external boundary of the input mesh
   * @param ref_inner_bdry_surf reference outmost layer (surface) points of the inner boundary layer
   * @param inner_peripheral_bias_terms terms describing the cumulative radial fractions of the
   * nodes within the inner boundary layer
   * @param azi_array list of azimuthal angles (in degrees) of the nodes on the input mesh's
   * external boundary for radius correction purpose
   * @param origin_pt centroid of the input mesh, which is used as the origin
   * @param points_array container to store all nodes' positions of the peripheral ring region
   */
  void innerBdryLayerNodesDefiner(const unsigned int input_ext_node_num,
                                  const std::vector<Real> input_bdry_angles,
                                  const std::vector<Point> ref_inner_bdry_surf,
                                  const std::vector<Real> inner_peripheral_bias_terms,
                                  const std::vector<Real> azi_array,
                                  const Point origin_pt,
                                  std::vector<std::vector<Point>> & points_array) const;
};
