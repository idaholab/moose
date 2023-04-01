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
 * Generates a tetrahedral (TET4) mesh from an input hexahedral (HEX8) mesh
 */
class HexToTetMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  HexToTetMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that comes from another generator
  std::unique_ptr<MeshBase> & _input;

  /// Mapping from original HEX8 nodes to eight TET4 element nodes
  std::vector<std::vector<unsigned int>> _tet4_nodes;

  /**
   * For each TET4 created, map from the original parent face ID
   * to the corresponding face of the daughter element. This is
   * needed to retain sideset assignments from the original HEX8
   * mesh in the created TET4 mesh.
   */
  std::vector<std::vector<std::pair<unsigned int, unsigned int>>> _daughter_face_to_parent_face;

  /// Number of TET4 elements per input HEX8 element
  const unsigned int TET4_ELEM_PER_HEX8 = 6;
};
