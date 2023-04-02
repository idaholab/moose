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

#include "libmesh/face_tri3.h"

/**
 * This XYMeshLineCutter object is designed to trim the input mesh by removing all the elements on
 * one side of a given straight line with special processing on the elements crossed by the cutting
 * line to ensure a smooth cross-section.
 */
class XYMeshLineCutter : public MeshGenerator
{
public:
  static InputParameters validParams();

  XYMeshLineCutter(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Method to cut the input mesh
  const enum class CutType { CUT_ELEM_TRI, MOV_NODE } _cutting_type;
  /// Name of the input mesh
  const MeshGeneratorName _input_name;
  /// Cutting line parameters, which are a, b, and c in line equation a*x+b*y+c=0. Note that a*x+b*y+c>0 part is being removed.
  const std::vector<Real> _cut_line_params;
  /// Boundary id to be assigned to the boundary formed by the cutting
  const boundary_id_type _new_boundary_id;
  /// Boundary id of the external boundary of the input mesh (only needed for MOV_NODE method)
  const boundary_id_type _input_mesh_external_boundary_id;
  /// IDs of the other boundaries that need to be conformed to during nodes moving (only needed for MOV_NODE method)
  const std::vector<boundary_id_type> _other_boundaries_to_conform;
  /// SubdomainName suffix used to rename the converted triangular elements
  const SubdomainName _tri_elem_subdomain_name_suffix;
  /// Customized id shift to define subdomain ids of the converted triangular elements
  const subdomain_id_type _tri_elem_subdomain_shift;
  /// Whether to improve TRI3 elements after CUT_ELEM_TRI method
  const bool _improve_tri_elements;
  /// Reference to input mesh pointer
  std::unique_ptr<MeshBase> & _input;
};
