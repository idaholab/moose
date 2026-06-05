//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/**
 * Generate a 2D layered mesh that represents a conformal boundary layer along the boundary of an
 * input 2D mesh or a 1D loop mesh.
 */
class XYTriangleBoundaryLayerGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  XYTriangleBoundaryLayerGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Input mesh defining the boundary on which to grow the boundary layer
  std::unique_ptr<MeshBase> & _input;

  /// The total thickness of the boundary layer to be created
  const Real _thickness;

  /// Number of element layers in the boundary layer
  const unsigned int _num_layers;

  /// Bias factor for the thickness of each layer
  const Real _layer_bias;

  /// Whether the boundary layer is grown outward or inward with respect to the input boundary
  const enum class BoundaryLayerDirection { OUTWARD, INWARD } _boundary_layer_direction;

  /// Optional subset of boundary names on the input mesh that define the seed boundary
  const std::vector<BoundaryName> _boundary_names;

  /// Optional boundary name for the interface between the generated boundary layer and the input mesh
  const BoundaryName _interface_name;

  /// Optional boundary name for the outer surface of the generated boundary layer
  const BoundaryName _surface_name;

  /// Optional subdomain name for the generated boundary layer
  const SubdomainName _subdomain_name;

  /// Optional subdomain id for the generated boundary layer
  const SubdomainID _output_subdomain_id;

  /// Triangular element type for the boundary layer
  const MooseEnum _tri_elem_type;

  /// Whether to keep the input mesh in the final output (stitched to the boundary layer)
  const bool _keep_input;
};
