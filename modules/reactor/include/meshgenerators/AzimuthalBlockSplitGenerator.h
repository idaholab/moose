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

/**
 * This AzimuthalBlockSplitGenerator object takes in a polygon/hexagon concentric circle mesh and
 * renames blocks on a user-defined azimuthal segment / wedge of the mesh.
 */
class AzimuthalBlockSplitGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  AzimuthalBlockSplitGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Input mesh to be modified
  const MeshGeneratorName _input_name;
  /// IDs to identify the input mesh blocks to be modified
  std::vector<subdomain_id_type> _old_block_ids;
  /// IDs of the new azimuthal blocks
  const std::vector<subdomain_id_type> _new_block_ids;
  /// Names of the new azimuthal blocks
  const std::vector<SubdomainName> _new_block_names;
  /// Whether volumes of ring regions need to be preserved
  const bool _preserve_volumes;
  /// Starting angular position of the modified azimuthal blocks
  Real _start_angle;
  /// Angular range of the modified azimuthal blocks
  const Real _angle_range;
  /// MeshMetaData: vector of all nodes' azimuthal angles
  std::vector<Real> & _azimuthal_angle_meta;
  /// Reference to input mesh pointer
  std::unique_ptr<MeshBase> & _input;
  /// MeshMetaData: number of mesh sectors of each polygon side
  std::vector<unsigned int> _num_sectors_per_side_meta;
  /// Ending angular position of the modified azimuthal blocks
  Real _end_angle;

  /**
   * Finds the azimuthal angles of the original mesh that need to be modified to match the edge
   * locations of the control drum's absorber region
   * @param terminal_angle azimuthal angle value at the starting or ending position
   * @param original_down reference to the azimuthal angle of the downer side of the element
   * intercepted by terminal_angle
   * @param original_down_it reference to iterator of the azimuthal angle vector pointing the
   * azimuthal angle of the downer side of the element intercepted by terminal_angle
   * @param original_up reference to the azimuthal angle of the upper side of the element
   * intercepted by terminal_angle
   * @param original_up_it reference to iterator of the azimuthal angle vector pointing the
   * azimuthal angle of the upper side of the element intercepted by terminal_angle
   */
  void angleIdentifier(const Real & terminal_angle,
                       Real & original_down,
                       std::vector<Real>::iterator & original_down_it,
                       Real & original_up,
                       std::vector<Real>::iterator & original_up_it);

  /**
   * Modifies the azimuthal angle to perform move the edge of the control drum during rotation
   * @param side_angular_shift azimuthal angle shift (needed when number of polygon sides is odd)
   * @param side_angular_range azimuthal angle range of each polygon side
   * @param azi_tol tolerance of the azimuthal angle
   * @param terminal_angle azimuthal angle value at the starting or ending position
   * @param original_down the azimuthal angle of the downer side of the element
   * intercepted by terminal_angle
   * @param original_down_it reference to iterator of the azimuthal angle vector pointing the
   * azimuthal angle of the downer side of the element intercepted by terminal_angle
   * @param original_up the azimuthal angle of the upper side of the element intercepted
   * by terminal_angle
   * @param original_up_it reference to iterator of the azimuthal angle vector pointing the
   * azimuthal angle of the upper side of the element intercepted by terminal_angle
   * @param azi_to_keep upper or lower side of the element that needs to be kept
   * @param azi_to_mod upper or lower side of the element that needs to be modified
   */
  void angleModifier(const Real & side_angular_shift,
                     const Real & side_angular_range,
                     const Real & azi_tol,
                     const Real & terminal_angle,
                     const Real & original_down,
                     std::vector<Real>::iterator & original_down_it,
                     const Real & original_up,
                     std::vector<Real>::iterator & original_up_it,
                     Real & azi_to_keep,
                     Real & azi_to_mod);

  /**
   * Modifies the nodes with the azimuthal angles modified in
   * AzimuthalBlockSplitGenerator::angleModifier()
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
   */
  void nodeModifier(ReplicatedMesh & mesh,
                    const std::vector<std::pair<Real, dof_id_type>> & node_id_mod,
                    const std::vector<std::pair<Real, dof_id_type>> & node_id_keep,
                    std::vector<Real> & circular_rad_list,
                    std::vector<Real> & non_circular_rad_list,
                    const std::vector<std::tuple<dof_id_type, boundary_id_type>> & node_list,
                    const Real & term_angle,
                    const bool & external_block_change,
                    const Real & rad_tol);
};
