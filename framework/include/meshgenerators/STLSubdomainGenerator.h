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
#include "MooseEnum.h"

class STLManifold;

/**
 * MeshGenerator for defining a subdomain based on whether element centroids lie within a closed
 * STL manifold.
 *
 * The generator is intentionally centroid-based to mirror the behavior of other subdomain tagging
 * mesh generators in MOOSE. The STL geometry is transformed in the order scale, then rotation,
 * then translation.
 */
class STLSubdomainGenerator : public MeshGenerator
{
public:
  /// Declare the input parameters for STL-based subdomain assignment.
  static InputParameters validParams();

  /// Construct the mesh generator from user input.
  STLSubdomainGenerator(const InputParameters & parameters);

  /// Apply STL-based subdomain tagging to the input mesh.
  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Input mesh to modify in place.
  std::unique_ptr<MeshBase> & _input;

  /// Whether to tag the interior or exterior of the STL manifold.
  const MooseEnum _location;

  /// Target subdomain identifier to assign.
  const subdomain_id_type _block_id;

  /// Whether retagging is limited to a subset of existing subdomains.
  const bool _has_restriction;

  /// STL file that defines the closed manifold.
  const FileName _stl_file;

  /// Per-axis scaling applied before rotation and translation.
  const RealVectorValue _scale;

  /// Extrinsic Euler rotation applied after scaling.
  const RealVectorValue _rotation;

  /// Translation applied after scaling and rotation.
  const RealVectorValue _translation;

  /// Absolute tolerance used by the manifold classifier.
  const Real _surface_tolerance;
};
