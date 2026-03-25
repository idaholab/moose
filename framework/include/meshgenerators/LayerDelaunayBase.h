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
 * Base class for mesh generators that needs 2D Delaunay triangulation-based layer mesh generation
 */
class LayerDelaunayBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  LayerDelaunayBase(const InputParameters & parameters);

protected:
  /**
   * Helper function to create a series of sub-mesh generators to generate a conformal boundary
   * layer mesh with Delaunay triangulation. The sub-mesh generators will be added to the
   * application and the name of the final mesh generator will be returned.
   * @param num_layers Number of layers in the boundary layer mesh
   * @param thickness Thickness of the boundary layer mesh
   * @param layer_bias Layer bias for the boundary layer mesh
   * @param is_outward_boundary_layer Whether the boundary layer is outward or inward
   * @param keep_input Whether to keep the input mesh in the final output mesh.
   * @param input_name The name of the input mesh generator
   * @param boundary_names The boundary names on the input mesh to which the boundary layer will be
   * applied. If empty, the method will identify the external boundary of the input mesh and apply
   * the boundary layer to it.
   * @param tri_elem_type The triangle element type for the boundary layer mesh.
   * @param block_id The subdomain id to use for the boundary layer mesh.
   * @param block_name The subdomain name to use for the boundary layer mesh.
   * @param innermost_boundary_id Optional boundary id to be assigned to the innermost boundary.
   * @param outermost_boundary_id Optional boundary id to be assigned to the outermost boundary.
   * @param name_suffix Optional suffix to add to the generated sub-mesh generator names to avert
   * name conflicts.
   * @returns The name of the final mesh generator that generates the boundary layer mesh, which can
   * be used for downstream processing.
   */
  MeshGeneratorName create_conformal_boundary_layer_mesh(
      const unsigned int num_layers,
      const Real thickness,
      const Real layer_bias,
      const bool is_outward_boundary_layer,
      const bool keep_input,
      const MeshGeneratorName & input_name,
      const std::vector<BoundaryName> & boundary_names,
      const MooseEnum & tri_elem_type,
      const SubdomainID & block_id = 0,
      const SubdomainName & block_name = SubdomainName(),
      const boundary_id_type innermost_boundary_id = libMesh::BoundaryInfo::invalid_id,
      const boundary_id_type outermost_boundary_id = libMesh::BoundaryInfo::invalid_id,
      const MeshGeneratorName & name_suffix = "");
};
