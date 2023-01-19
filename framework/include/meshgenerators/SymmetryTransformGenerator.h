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

/*
 * Applies a symmetry transformation to the mesh
 */
class SymmetryTransformGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SymmetryTransformGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// the input mesh
  std::unique_ptr<MeshBase> & _input;

  /// A point in the mirror plane from which to transform the mesh
  const RealEigenVector _mirror_point_vector;

  /// The normal of the mirror plane from which to transform the mesh
  RealEigenVector _mirror_normal_vector;
};
