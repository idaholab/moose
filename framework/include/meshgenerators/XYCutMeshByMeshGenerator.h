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
 * Cut a 2D XY input mesh using the outer boundary of another 2D mesh as the cut curve.
 * The cutter mesh's outer boundary must be a single simple closed polyline.
 */
class XYCutMeshByMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  XYCutMeshByMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// How to handle the two sides of the cutter polyline
  const enum class Mode { REMOVE_INSIDE, REMOVE_OUTSIDE, KEEP_BOTH } _mode;
  /// Method to cut crossed elements
  const enum class CutType { CUT_ELEM_POLY, CUT_ELEM_TRI } _cutting_type;
  /// Name of the primary input mesh
  const MeshGeneratorName _input_name;
  /// Name of the cutter input mesh
  const MeshGeneratorName _cutter_name;
  /// Boundary id assigned to the new cut interface
  const boundary_id_type _new_boundary_id;
  /// Tolerance used to snap primary nodes onto the cutter polyline (0 = no snap)
  const Real _snap_tol;
  /// Whether snapping is restricted to interior primary nodes
  const bool _snap_only_interior_nodes;
  /// Suffix used to name the C0POLYGON element subdomains (CUT_ELEM_POLY)
  const SubdomainName _poly_elem_subdomain_name_suffix;
  /// Suffix used to name the TRI3 element subdomains (CUT_ELEM_TRI)
  const SubdomainName _tri_elem_subdomain_name_suffix;
  /// Suffix for the inside portion when mode = KEEP_BOTH
  const SubdomainName _kept_inside_subdomain_name_suffix;
  /// Suffix for the outside portion when mode = KEEP_BOTH
  const SubdomainName _kept_outside_subdomain_name_suffix;
  /// Customized id shift to define subdomain ids of the new polygon/triangulated subdomains
  const subdomain_id_type _poly_subdomain_id_shift;
  /// Whether to improve TRI3 elements on the new cut boundary after CUT_ELEM_TRI
  const bool _improve_tri_elements;
  /// Reference to primary input mesh pointer
  std::unique_ptr<MeshBase> & _input;
  /// Reference to cutter input mesh pointer
  std::unique_ptr<MeshBase> & _cutter;
};
