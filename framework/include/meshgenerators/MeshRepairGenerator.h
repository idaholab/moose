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

/**
 * Mesh generator to perform various improvement / fixing operations on an input mesh
 */
class MeshRepairGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  MeshRepairGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

private:
  /// the input mesh
  std::unique_ptr<MeshBase> & _input;

  /// fixing mesh by deleting overlapping nodes
  const bool _fix_overlapping_nodes;
  /// tolerance for merging overlapping nodes
  const Real _node_overlap_tol;

  /// whether to flip element orientation such that they no longer have a negative volume
  const bool _fix_element_orientation;

  /// whether to split subdomains using each element's type
  const bool _elem_type_separation;

  /// Whether to merge boundaries with the same name but different ID
  const bool _boundary_id_merge;

  /// @brief Removes the elements with an volume value below the user threshold
  /// @param mesh the mesh to modify
  void removeSmallVolumeElements(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Removes nodes that overlap
  /// @param mesh the mesh to modify
  void fixOverlappingNodes(std::unique_ptr<MeshBase> & mesh) const;

  /// @brief Separate subdomain by element type because some output format (Exodus)
  ///        do not support mixed element types
  /// @param mesh the mesh to modify
  void separateSubdomainsByElementType(std::unique_ptr<MeshBase> & mesh) const;
};
