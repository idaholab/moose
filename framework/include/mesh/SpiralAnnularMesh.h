//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"

/**
 * Mesh generated from parameters
 */
class SpiralAnnularMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  SpiralAnnularMesh(const InputParameters & parameters);
  SpiralAnnularMesh(const SpiralAnnularMesh & /* other_mesh */) = default;

  // No copy
  SpiralAnnularMesh & operator=(const SpiralAnnularMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

protected:
  /// Radius of the inner circle
  const Real _inner_radius;

  /// Radius of the outer circle. Logically, it's bigger that inner_radius.
  const Real _outer_radius;

  /// Factor to increase initial_delta_r for each ring.
  /// For a uniform grid : radial_bias = 1.0
  Real _radial_bias;

  /// Number of nodes on each ring.
  const unsigned int _nodes_per_ring;

  /// Generate mesh of TRI6 elements instead of TRI3 elements.
  const bool _use_tri6;

  /// Number of rings.You can't specify
  /// both the number of rings and the radial bias if you want to match
  /// a specified outer radius exactly... you have to leave one of
  /// those parameters free so that it can be determined.
  unsigned int _num_rings;

  /// The boundary id to use for the cylinder.
  const boundary_id_type _cylinder_bid, _exterior_bid;

  // Width of the initial layer of elements around the cylinder.
  // This number should be approximately 2 * pi * inner_radius / nodes_per_ring
  // to ensure that the initial layer of elements is almost
  // equilateral
  const Real _initial_delta_r;
};
