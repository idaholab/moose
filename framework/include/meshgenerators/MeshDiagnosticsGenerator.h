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
 *
 */
class MeshDiagnosticsGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  MeshDiagnosticsGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// the input mesh
  std::unique_ptr<MeshBase> & _input;

  /// whether to check element volumes
  bool _check_element_volumes;
  /// counting number of small elements
  unsigned int _num_tiny_elems;
  /// counting number of big elements
  unsigned int _num_big_elems;
  /// minimum size for element volume to be counted as a tiny element
  Real _min_volume;
  /// maximum size for element volume to be counted as a big element
  Real _max_volume;
  /// whether to check different element types in the same sub-domain
  bool _check_element_types;
  /// whether to check for intersecting elements
  bool _check_element_overlap;
  /// whether to check for number of elements overlapping
  unsigned int _num_elem_overlaps;
  /// whether to check for elements in different planes (non_planar)
  bool _check_non_planar_sides;
  /// counting number of sides that are non-planar
  unsigned int _sides_non_planar;
  /// whether to check for non-conformal meshes
  bool _check_non_conformal_mesh;
  /// tolerance for meshes that are not comformal
  Real _non_conformality_tol;
  /// counting the number of non-conformal elements
  unsigned int _num_nonconformal_nodes;
  /// whether to check

  /// whether to check for the adaptivity of non-conformal meshes
  bool _check_adaptivity_non_conformality;
};
