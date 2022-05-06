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
 * This FillBetweenPointVectorsGenerator object is designed to generate a transition layer with
 * two sides containing different node numbers.
 */
class FillBetweenPointVectorsGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  FillBetweenPointVectorsGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Vector of Points of the first side
  const std::vector<Point> _positions_vector_1;
  /// Vector of Points of the second side
  const std::vector<Point> _positions_vector_2;
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
  /// Gaussian parameter used to smoothen local node density using Gaussian blurring
  const Real _sigma;
};
