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

/**
 * This ElementsToTetrahedronsConverter object is designed to convert all the elements in a 3D mesh
 * consisting of only linear elements into TET4 elements.
 */
class ElementsToTetrahedronsConverter : public MeshGenerator
{
public:
  static InputParameters validParams();

  ElementsToTetrahedronsConverter(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  /// An enum class for style of input polygon size
  enum class PointPlaneRelationIndex : short int
  {
    plane_normal_side = 1,
    opposite_plane_normal_side = -1,
    on_plane = 0
  };

protected:
  /// Name of the input mesh
  const MeshGeneratorName _input_name;
  /// Reference to input mesh pointer
  std::unique_ptr<MeshBase> & _input;
};
