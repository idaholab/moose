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

class TriangleManifold;

/**
 * MeshGenerator for defining a subdomain based on whether element vertex averages lie within a
 * closed manifold.
 *
 * The generator intentionally uses Elem::vertex_average() rather than the true geometric centroid
 * to mirror the inexpensive point-sampling behavior of other subdomain tagging mesh generators in
 * MOOSE.
 */
class ManifoldSubdomainGenerator : public MeshGenerator
{
public:
  /// Declare the input parameters for STL-based subdomain assignment.
  static InputParameters validParams();

  /// Construct the mesh generator from user input.
  ManifoldSubdomainGenerator(const InputParameters & parameters);

  /// Apply STL-based subdomain tagging using element vertex averages as the query points.
  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Input mesh to modify in place.
  std::unique_ptr<MeshBase> & _input;

  /// Surface mesh that defines the closed manifold.
  std::unique_ptr<MeshBase> & _manifold;

  /// Whether to tag the interior or exterior of the STL manifold.
  const MooseEnum _location;

  /// Target subdomain identifier to assign.
  const subdomain_id_type _block_id;

  /// Whether retagging is limited to a subset of existing subdomains.
  const bool _has_restriction;

  /// Absolute tolerance used by the manifold classifier; choose relative to geometry scale/noise.
  const Real _surface_tolerance;
};
