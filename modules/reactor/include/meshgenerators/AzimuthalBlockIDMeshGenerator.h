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
#include "MooseEnum.h"
#include "MeshMetaDataInterface.h"

/**
 * This AzimuthalBlockIDMeshGenerator object takes in a polygon/hexagon concentric circle mesh and
 * renames blocks on a user-defined azimuthal segment / wedge of the mesh.
 */
class AzimuthalBlockIDMeshGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  AzimuthalBlockIDMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Input mesh to be modified
  const MeshGeneratorName _input_name;
  /// IDs to identify the input mesh blocks to be modified
  std::vector<subdomain_id_type> _old_block_ids;
  /// Names to identify the input mesh blocks to be modified
  const std::vector<SubdomainName> _old_block_names;
  /// IDs of the new azimuthal blocks
  const std::vector<subdomain_id_type> _new_block_ids;
  /// Names of the new azimuthal blocks
  const std::vector<SubdomainName> _new_block_names;
  /// Whether volumes of ring resions need to be preserved
  const bool _preserve_volumes;
  /// Starting angular position of the modified azimuthal blocks
  Real _start_angle;
  /// Angular range of the modified azimuthal blocks
  const Real _angle_range;
  /// MeshMetaData: pitch size parameter of the hexagon
  Real & _pattern_pitch_meta;
  /// MeshMetaData: whether this mesh represents a control drum
  const bool _is_control_drum_meta;
  /// MeshMetaData: vector of all nodes' azimuthal angles
  std::vector<Real> & _azimuthal_angle_meta;
  /// Pointer to input mesh pointer
  std::unique_ptr<MeshBase> * _mesh_ptrs;
  /// Pointer to input mesh
  std::unique_ptr<ReplicatedMesh> _meshes;
  /// MeshMetaData: number of mesh sectors of each polygon side
  std::vector<unsigned int> _num_sectors_per_side_meta;
  /// Ending angular position of the modified azimuthal blocks
  Real _end_angle;

  /**
   * Finds the angles that need to be modified.
   * @param terminal_angle azimuthal angle value at the starting or ending position
   * @param original_down pointer to the azimuthal angle of the downer side of the element
   * intercepted by terminal_angle
   * @param original_down_it pointer to iterator of the azimuthal angle vector pointing the
   * azimuthal angle of the downer side of the element intercepted by terminal_angle
   * @param original_up pointer to the azimuthal angle of the upper side of the element intercepted
   * by terminal_angle
   * @param original_up_it pointer to iterator of the azimuthal angle vector pointing the azimuthal
   * angle of the upper side of the element intercepted by terminal_angle
   * @return n/a
   */
  void angleIndentifier(const Real terminal_angle,
                        Real * const original_down,
                        std::vector<Real>::iterator * const original_down_it,
                        Real * const original_up,
                        std::vector<Real>::iterator * const original_up_it);

  /**
   * Modifies the azimuthal angle identified by AzimuthalBlockIDMeshGenerator::angleIndentifier()
   * @param side_angular_shift azimuthal angle shift (needed when number of polygon sides is odd)
   * @param side_angular_range azimuthal angle range of each polygon side
   * @param azi_tol tolerance of the azimuthal angle
   * @param terminal_angle azimuthal angle value at the starting or ending position
   * @param original_down pointer to the azimuthal angle of the downer side of the element
   * intercepted by terminal_angle
   * @param original_down_it pointer to iterator of the azimuthal angle vector pointing the
   * azimuthal angle of the downer side of the element intercepted by terminal_angle
   * @param original_up pointer to the azimuthal angle of the upper side of the element intercepted
   * by terminal_angle
   * @param original_up_it pointer to iterator of the azimuthal angle vector pointing the azimuthal
   * angle of the upper side of the element intercepted by terminal_angle
   * @param azi_to_keep upper or downer side of the element that needs to be kept
   * @param azi_to_mod upper or downer side of the element that needs to be modified
   * @return n/a
   */
  void angleModifier(const Real side_angular_shift,
                     const Real side_angular_range,
                     const Real azi_tol,
                     const Real terminal_angle,
                     Real * const original_down,
                     std::vector<Real>::iterator * const original_down_it,
                     Real * const original_up,
                     std::vector<Real>::iterator * const original_up_it,
                     Real * const azi_to_keep,
                     Real * const azi_to_mod);

  /**
   * Modifies the nodes with the azimuthal angles modified in
   * AzimuthalBlockIDMeshGenerator::angleModifier()
   * @param mesh input mesh of which the nodes need to be modified
   * @param node_id_mod vector of node_ids that need to be modified along with the corresponding
   * radii
   * @param node_id_keep vector of node_ids that need to be kept along with the corresponding radii
   * @param circular_rad_list vector to record the radii that belong to the nodes in the circular
   * regions
   * @param non_circular_rad_list vector to record the radii that belong to the nodes in the
   * non-circular regions
   * @param node_list nodeset of the external boundary
   * @param term_angle azimuthal angle value at the starting or ending position
   * @param external_block_change whether the nodes of the outermost block need to be modified
   * @param rad_tol tolerance of the radii of nodes
   * @return a mesh with nodes modified to the new azimuthal positions
   */
  std::unique_ptr<ReplicatedMesh>
  nodeModifier(std::unique_ptr<ReplicatedMesh> mesh,
               const std::vector<std::pair<Real, dof_id_type>> node_id_mod,
               const std::vector<std::pair<Real, dof_id_type>> node_id_keep,
               std::vector<Real> * const circular_rad_list,
               std::vector<Real> * const non_circular_rad_list,
               const std::vector<std::tuple<dof_id_type, boundary_id_type>> node_list,
               const Real term_angle,
               const bool external_block_change,
               const Real rad_tol);
};
