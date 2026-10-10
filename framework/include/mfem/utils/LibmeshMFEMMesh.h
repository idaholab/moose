//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "libmesh/id_types.h"
#include "libmesh/utility.h"
#include "LibmeshMFEMBlockInfo.h"
#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

#include <array>
#include <map>
#include <string>
#include <vector>

/**
 * LibmeshMFEMMesh
 *
 * An mfem::Mesh built from the data describing a libMesh mesh.
 */
class LibmeshMFEMMesh : public mfem::Mesh
{
public:
  using dof_id_type = libMesh::dof_id_type;
  using subdomain_id_type = libMesh::subdomain_id_type;
  using boundary_id_type = libMesh::boundary_id_type;

  /// libMesh IDs of the elements in each block
  using ElementIDsForBlockID = std::map<subdomain_id_type, std::vector<dof_id_type>>;
  /// libMesh IDs of the nodes of each element
  using NodeIDsForElementID = std::map<dof_id_type, std::vector<dof_id_type>>;
  /// libMesh IDs of the nodes of each element side on each boundary
  using NodeIDsForBoundaryID = std::map<boundary_id_type, std::vector<std::vector<dof_id_type>>>;
  /// Local indices of the element sides on each boundary
  using SideIDsForBoundaryID = std::map<boundary_id_type, std::vector<unsigned int>>;
  /// Blocks of the elements whose sides are on each boundary
  using BlockIDsForBoundaryID = std::map<boundary_id_type, std::vector<subdomain_id_type>>;
  /// Coordinates of each libMesh node
  using CoordinatesForNodeID = std::map<dof_id_type, std::array<double, 3>>;

  /**
   * Build the MFEM mesh from the libMesh mesh data. Higher-order elements are represented by
   * setting the nodes of the mesh.
   */
  LibmeshMFEMMesh(const int num_elements_in_mesh,
                  const LibmeshMFEMBlockInfo & block_info,
                  const std::vector<subdomain_id_type> & unique_block_ids,
                  const std::map<subdomain_id_type, std::string> & block_ids_to_names,
                  const std::vector<boundary_id_type> & unique_side_boundary_ids,
                  const std::map<boundary_id_type, std::string> & bound_ids_to_names,
                  const std::vector<dof_id_type> & unique_libmesh_corner_node_ids,
                  const ElementIDsForBlockID & libmesh_element_ids_for_block_id,
                  const NodeIDsForElementID & libmesh_node_ids_for_element_id,
                  const NodeIDsForBoundaryID & libmesh_node_ids_for_boundary_id,
                  const SideIDsForBoundaryID & libmesh_side_ids_for_boundary_id,
                  const BlockIDsForBoundaryID & libmesh_block_ids_for_boundary_id,
                  const CoordinatesForNodeID & coordinates_for_libmesh_node_id);

private:
  /**
   * Calls buildMFEMVertices, buildMFEMElements, buildMFEMBoundaryElements methods
   * to construct the mesh. NB: - additional methods should be called after this
   * to handle second-order elements. The Finalize() method must be called at the
   * end.
   */
  void
  buildMFEMVerticesAndElements(const int num_elements_in_mesh,
                               const LibmeshMFEMBlockInfo & block_info,
                               const std::vector<subdomain_id_type> & unique_block_ids,
                               const std::map<subdomain_id_type, std::string> & block_ids_to_names,
                               const std::vector<boundary_id_type> & unique_side_boundary_ids,
                               const std::map<boundary_id_type, std::string> & bound_ids_to_names,
                               const std::vector<dof_id_type> & unique_libmesh_corner_node_ids,
                               const ElementIDsForBlockID & libmesh_element_ids_for_block_id,
                               const NodeIDsForElementID & libmesh_node_ids_for_element_id,
                               const NodeIDsForBoundaryID & libmesh_node_ids_for_boundary_id,
                               const SideIDsForBoundaryID & libmesh_side_ids_for_boundary_id,
                               const BlockIDsForBoundaryID & libmesh_block_ids_for_boundary_id,
                               const CoordinatesForNodeID & coordinates_for_libmesh_node_id);

  /**
   * Sets the protected variable array using the provided vector of corner node
   * IDs from MOOSE. Note that the vertices (named "nodes" in MOOSE) are ONLY
   * at the corners of elements. These are referred to as "corner nodes" in MOOSE.
   */
  void buildMFEMVertices(const std::vector<dof_id_type> & unique_libmesh_corner_node_ids,
                         const CoordinatesForNodeID & coordinates_for_libmesh_node_id);

  /**
   * Construct the MFEM elements array.
   */
  void buildMFEMElements(const int num_elements_in_mesh,
                         const LibmeshMFEMBlockInfo & block_info,
                         const std::vector<subdomain_id_type> & unique_block_ids,
                         const std::map<subdomain_id_type, std::string> & block_ids_to_names,
                         const ElementIDsForBlockID & libmesh_element_ids_for_block_id,
                         const NodeIDsForElementID & libmesh_node_ids_for_element_id);

  /**
   * Construct the boundary array of elements.
   */
  void buildMFEMBoundaryElements(const LibmeshMFEMBlockInfo & block_info,
                                 const std::vector<boundary_id_type> & unique_side_boundary_ids,
                                 const std::map<boundary_id_type, std::string> & bound_ids_to_names,
                                 const NodeIDsForBoundaryID & libmesh_node_ids_for_boundary_id,
                                 const SideIDsForBoundaryID & libmesh_side_ids_for_boundary_id,
                                 const BlockIDsForBoundaryID & libmesh_block_ids_for_boundary_ids);

  /**
   * Returns a pointer to an mfem::Element.
   */
  mfem::Element *
  buildMFEMElement(const int element_type, const int * vertex_ids, const int block_id);

  /**
   * Returns an pointer to an mfem::Element (for faces only).
   */
  mfem::Element *
  buildMFEMFaceElement(const int face_type, const int * vertex_ids, const int boundary_id);

  /**
   * Called internally in constructor if the element is second-order or higher.
   */
  void handleHigherOrderFESpace(const LibmeshMFEMBlockInfo & block_info,
                                const std::vector<subdomain_id_type> & unique_block_ids,
                                const ElementIDsForBlockID & libmesh_element_ids_for_block_id,
                                const NodeIDsForElementID & libmesh_node_ids_for_element_id,
                                const CoordinatesForNodeID & coordinates_for_libmesh_node_id);

  /**
   * Verifies whether the libmesh and mfem node ids have a unique mapping. All
   * coordinates should match and every mfem node id should have a corresponding
   * libmesh node id. Any left-over node ids will be detected.
   */
  void verifyUniqueMappingBetweenLibmeshAndMFEMNodes(
      const LibmeshMFEMBlockInfo & block_info,
      const NodeIDsForElementID & libmesh_node_ids_for_element_id,
      const CoordinatesForNodeID & coordinates_for_libmesh_node_id,
      const std::map<int, dof_id_type> & libmesh_node_id_for_mfem_node_id);

  /**
   * Accessors.
   */
  inline int getMFEMElementID(dof_id_type libmesh_element_id) const
  {
    return libmesh_map_find(_mfem_element_id_for_libmesh_element_id, libmesh_element_id);
  }

  inline int getMFEMVertexIndex(dof_id_type libmesh_corner_node_id) const
  {
    return libmesh_map_find(_mfem_vertex_index_for_libmesh_corner_node_id, libmesh_corner_node_id);
  }

  std::map<dof_id_type, int> _mfem_element_id_for_libmesh_element_id;
  std::map<dof_id_type, int> _mfem_vertex_index_for_libmesh_corner_node_id;
};

#endif
