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
 * This FillBetweenCurvesGenerator object is designed to generate a transition layer to connect
 * two curves.
 */
class FillBetweenCurvesGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  FillBetweenCurvesGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Name of the mesh which contains the first input boundary
  const MeshGeneratorName _input_name_1;
  /// Name of the mesh which contains the second input boundary
  const MeshGeneratorName _input_name_2;
  /// Translation applied to the first input mesh
  const Point _mesh_1_shift;
  /// Translation applied to the second input mesh
  const Point _mesh_2_shift;
  /// Number of sublayers of the mesh to be generated
  const unsigned int _num_layers;
  /// Subdomain ID to be assigned to the generated transition layer
  const subdomain_id_type _block_id;
  /// ID to be assigned to the boundary that corresponds to the input boundary on the first input mesh
  const boundary_id_type _input_boundary_1_id;
  /// ID to be assigned to the boundary that corresponds to the input boundary on the second input mesh
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
  /// The mesh which contains the first input boundary
  std::unique_ptr<MeshBase> & _input_1;
  /// The mesh which contains the second input boundary
  std::unique_ptr<MeshBase> & _input_2;

  /**
   * Calculates the centroid of a curve mesh.
   * @param curve input mesh that contains the curve whose centroid needs to be calculated
   * @return a Point data containing the curve centroid
   */
  Point curveCentroidPoint(const ReplicatedMesh & curve);
};
