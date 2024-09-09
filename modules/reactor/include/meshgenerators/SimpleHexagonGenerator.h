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

/**
 * This SimpleHexagonGenerator object is designed to generate a simple hexagonal mesh that only
 * contains six simple azimuthal triangle slices.
 */
class SimpleHexagonGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  SimpleHexagonGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Type of elements to build the mesh
  const enum class ElemType { TRI, QUAD, HYBRID } _element_type;
  /// Size parameter of the hexagon
  const Real _hexagon_size;
  /// Style of the size parameter
  const PolygonSizeStyle _hexagon_size_style;
  /// Pitch size of the hexagon
  Real _pitch;
  /// Number of radial meshing intervals
  const unsigned int _radial_intervals;
  /// Subdomain ID of the mesh
  const std::vector<subdomain_id_type> _block_id;
  /// Subdomain Name of the mesh
  const std::vector<SubdomainName> _block_name;
  /// Whether external_boundary_id is provided
  const bool _boundary_id_valid;
  /// Boundary ID of the external boundary
  const boundary_id_type _external_boundary_id;
  /// Boundary Name of the external boundary
  const BoundaryName _external_boundary_name;
};
