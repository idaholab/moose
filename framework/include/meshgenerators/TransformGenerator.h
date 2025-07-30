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

/*
 * A mesh generator that applies a linear transformation (rotation, translation) to the mesh
 */
class TransformGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  TransformGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// the input mesh
  std::unique_ptr<MeshBase> & _input;
  /// the transform to apply to the mesh
  const MooseEnum _transform;

  /// @brief Rotate the mesh using extrinsic rotation with given angles. See https://en.wikipedia.org/wiki/Euler_angles, Proper Euler Angles 'ZXZ'.
  void rotateExtrinsic(MeshBase & mesh, const Real alpha, const Real beta, const Real gamma);

  /// @brief Rotate a mesh using a given rotation matrix. See above Wiki page for various formulae.
  void rotateWithMatrix(MeshBase & mesh, const GenericRealTensorValue<false> & rotation_matrix);
};
