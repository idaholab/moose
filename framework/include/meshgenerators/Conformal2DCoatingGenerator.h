//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LayerDelaunayBase.h"

/**
 * Generate a 2D layered mesh that represents a conformal coating along the boundary of an input 2D
 * mesh or a 1D loop mesh.
 */
class Conformal2DCoatingGenerator : public LayerDelaunayBase
{
public:
  static InputParameters validParams();

  Conformal2DCoatingGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Name of input mesh
  const MeshGeneratorName & _input_name;

  /// The thickness of the coating to be created
  const Real _thickness;

  /// Number of layers to be created in the coating
  const unsigned int _num_layers;

  /// The bias factor for the thickness of each layer
  const Real _layer_bias;

  /// The direction in which the coating is created with respect to the boundary of the input mesh
  const enum class CoatingDirection { OUTWARD, INWARD } _coating_direction;

  /// The boundary names around which the coating will be created
  const std::vector<BoundaryName> _boundary_names;

  /// The optional boundary name to be assigned to the interface between the generated coating and the input mesh
  const BoundaryName _interface_name;

  /// The optional boundary name to be assigned to the surface of the generated coating
  const BoundaryName _surface_name;

  /// The optional subdomain name to be assigned to the generated coating mesh
  const SubdomainName _subdomain_name;

  /// The optional subdomain ID to be assigned to the generated coating mesh
  const SubdomainID _output_subdomain_id;

  /// The type of triangular elements to use for the coating
  const MooseEnum _tri_elem_type;

  /// Whether to keep the input mesh in the final output
  const bool _keep_input;

  /// mesh pointer to get the subgenerator output
  std::unique_ptr<MeshBase> * _build_mesh;
};
