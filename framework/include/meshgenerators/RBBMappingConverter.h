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
 * Takes a mesh with a Lagrange mapping, and reinterpolate its nodes
 * using a Rational Bezier-Bernstein mapping of the same order.
 *
 * For certain types of meshes (e.g. circular arcs, cylindrical
 * extrusions, quad shells between latitudinal/longitudinal boundary
 * arcs) the resulting RBB mapping is IsoGeometric; i.e. the domain
 * which was only approximated by the Lagrange interpolant is
 * represented exactly (to within floating-point error) by the RBB
 * interpolant.
 */
class RBBMappingConverter : public MeshGenerator
{
public:
  static InputParameters validParams();

  RBBMappingConverter(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Input mesh defining the original mixed mesh
  std::unique_ptr<MeshBase> & _input_ptr;
};
