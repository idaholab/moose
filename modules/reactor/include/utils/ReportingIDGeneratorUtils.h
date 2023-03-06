//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "libmesh/elem.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/dof_object.h"

namespace ReportingIDGeneratorUtils
{
/**
 * Enum item for reporting id assign types.
 */
enum class AssignType
{
  /// assign unique IDs for each tile in the lattice in sequential order
  cell,
  /// assign the same reporting IDs for all tiles in the pattern with same input
  pattern,
  /// assign IDs based on user-defined mapping
  manual
};

/**
 * assign IDs for each component in pattern in sequential order
 * @param meshes input meshes of the cartesian or hexagonal patterned mesh generator
 * @param pattern 2D vector of the mesh pattern
 * @param use_exclude_id flag to indicate if exclude_id is defined
 * @param exclude_ids flag to indicate if exclude_id is used for each input mesh
 * @return list of reporting IDs for individual mesh elements
 **/
std::vector<dof_id_type>
getCellwiseIntegerIDs(const std::vector<std::unique_ptr<ReplicatedMesh>> & meshes,
                      const std::vector<std::vector<unsigned int>> & pattern,
                      const bool use_exclude_id,
                      const std::vector<bool> & exclude_ids);

/**
 * assign IDs for each input component type
 * @param meshes input meshes of the cartesian or hexagonal patterned mesh generator
 * @param pattern 2D vector of the mesh pattern
 * @return list of reporting IDs for individual mesh elements
 **/
std::vector<dof_id_type>
getPatternIntegerIDs(const std::vector<std::unique_ptr<ReplicatedMesh>> & meshes,
                     const std::vector<std::vector<unsigned int>> & pattern);

/**
 * assign IDs based on user-defined mapping defined in id_pattern
 * @param meshes input meshes of the cartesian or hexagonal patterned mesh generator
 * @param pattern 2D vector of the mesh pattern
 * @param id_pattern user-defined integer ID for each input pattern cell
 * @return list of reporting IDs for individual mesh elements
 **/
std::vector<dof_id_type>
getManualIntegerIDs(const std::vector<std::unique_ptr<ReplicatedMesh>> & meshes,
                    const std::vector<std::vector<unsigned int>> & pattern,
                    const std::vector<std::vector<dof_id_type>> & id_pattern);

/**
 * get list of block IDs in input mesh cells
 * @param meshes input meshes of the cartesian or hexagonal patterned mesh generator
 * @param pattern 2D vector of the mesh pattern
 * @return list of  block IDs in input meshes
 **/
std::set<SubdomainID> getCellBlockIDs(const std::vector<std::unique_ptr<ReplicatedMesh>> & meshes,
                                      const std::vector<std::vector<unsigned int>> & pattern);

/**
 * get list of block IDs for the assembly duck regions
 * @param mesh  output mesh from the cartesian or hexagonal patterned mesh generator
 * @param has_assembly_boundary flag to indicate if assembly boundary exists
 * @param background_blk_ids list of block ID assigned to background regions
 * @param blks list of block defined in the input meshes of the cartesian or hexagonal patterned
 *mesh generator
 * @return list of block ids in the assembly duct region
 **/
std::map<SubdomainID, unsigned int>
getDuckBlockIDs(const std::unique_ptr<MeshBase> & mesh,
                const bool has_assembly_boundary,
                const std::set<subdomain_id_type> background_blk_ids,
                const std::set<SubdomainID> & blks);

/**
 * assign the reporting IDs to the output mesh from the cartesian or hexagonal patterned mesh
 *generator
 * @param mesh  output mesh from the cartesian or hexagonal patterned mesh generator
 * @param extra_id_index index of extra integer id for assigning the reproting IDs
 * @param assign_type type of integer ID assignment
 * @param use_exclude_id flag to indicate if exclude_id is defined
 * @param exclude_ids flag to indicate if exclude_id is used for each input mesh
 * @param has_assembly_boundary flag to indicate if assembly boundary exists
 * @param background_block_ids list of block ID assigned to background regions
 * @param input_meshes input meshes of the cartesian or hexagonal patterned mesh generator
 * @param pattern 2D vector of the mesh pattern
 * @param id_pattern user-defined integer ID for each input pattern cell
 * @return output mesh file having reporting IDs
 **/
void assignReportingIDs(std::unique_ptr<MeshBase> & mesh,
                        const unsigned int extra_id_index,
                        const ReportingIDGeneratorUtils::AssignType assign_type,
                        const bool use_exclude_id,
                        const std::vector<bool> & exclude_ids,
                        const bool has_assembly_boundary,
                        const std::set<subdomain_id_type> background_block_ids,
                        const std::vector<std::unique_ptr<ReplicatedMesh>> & input_meshes,
                        const std::vector<std::vector<unsigned int>> & pattern,
                        const std::vector<std::vector<dof_id_type>> & id_pattern);
}
