//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolygonMeshGeneratorBase.h"

/**
 * This TriPinHexAssemblyGenerator object is a base class to be inherited for polygon
 * mesh generators.
 */
class TriPinHexAssemblyGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  TriPinHexAssemblyGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Radii of concentric circles in the three diamond sections
  const std::vector<std::vector<Real>> _ring_radii;
  /// Numbers of radial layers in each ring region in the three diamond sections
  const std::vector<std::vector<unsigned int>> _ring_intervals;
  /// Block ids of the ring regions in the three diamond sections
  const std::vector<std::vector<subdomain_id_type>> _ring_block_ids;
  /// Block names of the ring regions in the three diamond sections
  const std::vector<std::vector<SubdomainName>> _ring_block_names;
  /// Type of hexagon_size parameter
  const PolygonSizeStyle _hexagon_size_style;
  /// Length of the side of the hexagon
  const Real _side_length;
  /// Offset distance of the circle centers from the center of each diamond section
  const Real _ring_offset;
  /// Whether the radii need to be corrected for polygonization during meshing
  const bool _preserve_volumes;
  /// Assembly orientation option
  const enum class AssmOrient { pin_up, pin_down } _assembly_orientation;
  /// Mesh sector number of each polygon side
  const unsigned int _num_sectors_per_side;
  /// Numbers of radial intervals of the background regions
  const unsigned int _background_intervals;
  /// Block ids of the background region
  const std::vector<subdomain_id_type> _background_block_ids;
  /// Block names of the background region
  const std::vector<SubdomainName> _background_block_names;
  /// Boundary ID of mesh's external boundary
  const boundary_id_type _external_boundary_id;
  /// Boundary name of mesh's external boundary
  const std::string _external_boundary_name;
  /// Name of extra integer ID to be assigned to each of the three pin domains
  const std::string _pin_id_name;
  /// Values of extra integer ID to be assigned to each of the three pin domains
  const std::vector<dof_id_type> _pin_id_values;
  /// MeshMetaData: maximum node id of the background region
  dof_id_type & _node_id_background_meta;
  /// Whether the generated mesh contains ring regions
  std::vector<bool> _has_rings;

  /**
   * Generates a single-pin diamond section mesh, which is one-third of the triple-pin hexagonal
   * assembly mesh.
   * @param side_length side length of the hexagon assembly to be generated.
   * @param ring_offset offset of the center of the ring region from the center of the diamond
   * @param ring_radii radii of major concentric circles in the diamond section
   * @param ring_intervals number of radial mesh intervals of the major concentric circles
   * @param has_rings whether the section contains any ring regions
   * @param preserve_volumes whether the radii of the rings are modified to preserve volumes
   * @param num_sectors_per_side number of azimuthal sectors per side
   * @param background_intervals number of radial meshing intervals in background region (area
   * outside the rings)
   * @param block_ids_new customized block ids for the regions
   * @param node_id_background_meta reference to the first node's id of the background region
   * @return a mesh of a single-pin diamond section mesh
   */
  std::unique_ptr<ReplicatedMesh>
  buildSinglePinSection(const Real side_length,
                        const Real ring_offset,
                        const std::vector<Real> ring_radii,
                        const std::vector<unsigned int> ring_intervals,
                        const bool has_rings,
                        const bool preserve_volumes,
                        const unsigned int num_sectors_per_side,
                        const unsigned int background_intervals,
                        const std::vector<subdomain_id_type> block_ids_new,
                        dof_id_type & node_id_background_meta);
};
