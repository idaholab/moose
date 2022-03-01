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
};
