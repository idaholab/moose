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
#include "ReportingIDGeneratorUtils.h"
#include "MooseEnum.h"
#include "MeshMetaDataInterface.h"
#include "ReportingIDGeneratorUtils.h"

/**
 * This PatternedCartesianMeshGenerator source code assembles square meshes into a rectangular grid
 * and optionally adds a duct around the grid.
 */
class PatternedCartesianMeshGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  PatternedCartesianMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// The input meshes
  const std::vector<std::unique_ptr<MeshBase> *> _mesh_ptrs;
  /// Names of input meshes
  const std::vector<MeshGeneratorName> & _input_names;
  /// 2D vector of the square pattern
  const std::vector<std::vector<unsigned int>> & _pattern;
  /// Type of the external boundary shape
  const MooseEnum _pattern_boundary;
  /// Whether a reactor core mesh with core metadata is generated
  const bool _generate_core_metadata;
  /// Number of radial intervals in the background region
  const unsigned int _background_intervals;
  /// Whether the square pattern has external duct(s)
  const bool _has_assembly_duct;
  /// Size parameter(s) of duct(s)
  std::vector<Real> _duct_sizes;
  /// Style of the duct size parameter(s)
  const PolygonSizeStyle _duct_sizes_style;
  /// Number(s) of radial intervals of duct layer(s)
  const std::vector<unsigned int> _duct_intervals;
  /// Whether the nodes on the external boundary are uniformly distributed
  const bool _uniform_mesh_on_sides;
  /// Whether a text file containing control drum positions is generated
  const bool _generate_control_drum_positions_file;
  /// Wheter control drum IDs are assigned as an extra element integer
  const bool _assign_control_drum_id;
  /// The mesh rotation angle after mesh generation
  const Real _rotate_angle;
  /// Subdomain IDs of the duct layers
  const std::vector<subdomain_id_type> _duct_block_ids;
  /// Subdomain Names of the duct layers
  const std::vector<SubdomainName> _duct_block_names;
  /// Boundary ID of mesh's external boundary
  const boundary_id_type _external_boundary_id;
  /// Boundary name of mesh's external boundary
  const std::string _external_boundary_name;
  /// Whether inward interface boundaries are created
  const bool _create_inward_interface_boundaries;
  /// Whether outward interface boundaries are created
  const bool _create_outward_interface_boundaries;
  /// Whether the non-circular region (outside the rings) can be deformed
  const bool _deform_non_circular_region;
  /// Pitch size of the input assembly mesh
  Real _pattern_pitch;
  /// Subdomain IDs of the peripheral regions
  std::vector<subdomain_id_type> _peripheral_block_ids;
  /// Subdomain Names of the peripheral regions
  std::vector<SubdomainName> _peripheral_block_names;
  /// Whether reporting ID is added to mesh
  const bool _use_reporting_id;
  /// names of reporting ID
  std::vector<std::string> _reporting_id_names;
  /// reporting ID assignment type
  std::vector<ReportingIDGeneratorUtils::AssignType> _assign_types;
  /// flag to indicate if exclude_id is defined
  const bool _use_exclude_id;
  /// vector indicating which ids in the pattern to exclude (true at pattern positions to exclude)
  std::vector<bool> _exclude_ids;
  /// hold ID patterns for each manual reporting ID. Individual ID pattern contains ID values for each pattern cell.
  std::map<std::string, std::vector<std::vector<dof_id_type>>> _id_patterns;
  /// whether the interface boundary ids from input meshes are shifted, using a user-defined pattern of values for each pattern cell
  const bool _use_interface_boundary_id_shift;
  /// hold user-defined shift values for each pattern cell
  std::vector<std::vector<boundary_id_type>> _interface_boundary_id_shift_pattern;
  /// Type of quadrilateral elements to be generated in the periphery region
  QUAD_ELEM_TYPE _boundary_quad_elem_type;

  /**
   * Adds background and duct region mesh to each part outer part of stitched square meshes. Note
   * that the function works for single unit square mesh (corner or edge) separately before
   * stitching.
   * @param mesh input mesh to add the peripheral region onto
   * @param pattern index of the input mesh for patterning
   * @param pitch pitch size of the input mesh
   * @param extra_dist extra distances from inner boundary to define background and ducts layer
   * locations that are needed to create the peripheral region
   * @param num_sectors_per_side_array numbers of azimuthal intervals of all input unit meshes
   * @param peripheral_duct_intervals numbers of radial intervals of the duct regions
   * @param rotation_angle angle that the generated mesh will be rotated by
   * @param mesh_type whether the peripheral region is for a corner or a side of a patterned mesh
   * @return a mesh of the cartesian pattern mesh with peripheral region added.
   */
  void addPeripheralMesh(ReplicatedMesh & mesh,
                         const unsigned int pattern, //_pattern[i][j]
                         const Real pitch,           // pitch_array.front()
                         const std::vector<Real> & extra_dist,
                         const std::vector<unsigned int> & num_sectors_per_side_array,
                         const std::vector<unsigned int> & peripheral_duct_intervals,
                         const Real rotation_angle,
                         const unsigned int mesh_type);

  /**
   * Computes the inner and outer node positions of the peripheral region for a single layer.
   * @param positions_inner key positions (i.e., vertices and mid-points) of the inner side of the
   * peripheral region
   * @param d_positions_outer key incremental positions (i.e., vertices and mid-points) of the outer
   * side of the peripheral region
   * @param extra_dist_in extra distance applied to the inner side of the peripheral layer
   * @param extra_dist_out extra distance applied to the outer side
   * @param pitch pitch size of the involved polygonal mesh
   */
  void positionSetup(std::vector<std::pair<Real, Real>> & positions_inner,
                     std::vector<std::pair<Real, Real>> & d_positions_outer,
                     const Real extra_dist_in,
                     const Real extra_dist_out,
                     const Real pitch) const;
  /**
   * Adds the reporting IDs onto the input mesh.
   * @param  mesh input mesh to add the reporting IDs onto
   * @param from_meshes meshes to take reporting IDs from
   */
  void addReportingIDs(MeshBase & mesh,
                       const std::vector<std::unique_ptr<ReplicatedMesh>> & from_meshes) const;
};
