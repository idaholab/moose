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
  /// counting number of overlapped nodes fixed
  unsigned int _num_fixed_nodes;

  /// refining elements that are too big
  const bool _fix_max_element_size;
  /// maximum element size over which elements are refined
  const Real _max_element_size;
  /// counting the number of elements that need to be refined for being too big
  unsigned int _num_refined_elems;

  /// whether to flip elements that are oriented such that they have a negative volume
  const bool _fix_element_orientation;
};
