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

#include "libmesh/point.h"

// Forward declarations
class FancyExtruderGenerator;

template <>
InputParameters validParams<FancyExtruderGenerator>();

/**
 * Extrudes a mesh to another dimension
 */
class FancyExtruderGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  FancyExtruderGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that comes from another generator
  std::unique_ptr<MeshBase> & _input;

  /// Height of each elevation
  const std::vector<Real> & _heights;

  /// Number of layers in each elevation
  const std::vector<unsigned int> & _num_layers;

  /// Subdomains to swap out for each elevation
  const std::vector<std::vector<subdomain_id_type>> & _subdomain_swaps;

  /// Easier to work with version of _sudomain_swaps
  std::vector<std::unordered_map<subdomain_id_type, subdomain_id_type>> _subdomain_swap_pairs;

  /// The direction of the extrusion
  Point _direction;

  bool _has_top_boundary;
  boundary_id_type _top_boundary;

  bool _has_bottom_boundary;
  boundary_id_type _bottom_boundary;
};
