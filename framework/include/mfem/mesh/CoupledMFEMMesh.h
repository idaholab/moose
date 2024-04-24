//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "ExclusiveMFEMMesh.h"
#include "CubitElementInfo.h"
#include "libmesh/face_quad4.h"

/**
 * CoupledMFEMMesh
 */
class CoupledMFEMMesh : public ExclusiveMFEMMesh
{
public:
  static InputParameters validParams();

  CoupledMFEMMesh(const InputParameters & parameters);

  virtual ~CoupledMFEMMesh();

  std::unique_ptr<MooseMesh> safeClone() const override;

  /**
   * Builds only the MOOSE mesh from the file.
   */
  void buildMesh() override;

  /**
   * Override method in ExclusiveMFEMMesh.
   */
  inline int getLocalMFEMNodeId(const int libmesh_global_node_id) const override
  {
    return _mfem_local_node_id_for_libmesh_global_node_id.at(libmesh_global_node_id);
  }

  inline int getLibmeshGlobalNodeId(const int mfem_local_node_id) const override
  {
    return _libmesh_global_node_id_for_mfem_local_node_id.at(mfem_local_node_id);
  }

protected:
  /**
   * An internal method used to create maps from each boundary ID to vectors of side IDs
   * and element IDs.
   */
  void buildBoundaryInfo(std::map<int, std::vector<int>> & element_ids_for_boundary_id,
                         std::map<int, std::vector<int>> & side_ids_for_boundary_id);

  /**
   * Create a mapping from each boundary ID to a vector of vectors containing the global node
   * IDs of nodes that lie on the faces of elements that fall on the boundary.
   */
  void buildBoundaryNodeIDs(
      const std::vector<int> & unique_side_boundary_ids,
      const std::map<int, std::vector<int>> & element_ids_for_boundary_id,
      const std::map<int, std::vector<int>> & side_ids_for_boundary_id,
      std::map<int, std::vector<std::vector<unsigned int>>> & node_ids_for_boundary_id);

  /**
   * Builds two maps:
   * 1. Mapping from each block ID --> vector containing all element IDs for block.
   * 2. Mapping from each element --> vector containing all global node IDs for element.
   */
  void buildElementAndNodeIDs(const std::vector<int> & unique_block_ids,
                              std::map<int, std::vector<int>> & element_ids_for_block_id,
                              std::map<int, std::vector<int>> & node_ids_for_element_id);

  /**
   * Iterates through each block to find the elements in the block. For each
   * element in a block, it runs through the nodes in the block and adds only
   * the corner nodes to a vector. This is then sorted and only unique global
   * node IDs are retained.
   */
  void buildUniqueCornerNodeIDs(std::vector<int> & unique_corner_node_ids,
                                const std::vector<int> & unique_block_ids,
                                const std::map<int, std::vector<int>> & element_ids_for_block_id,
                                const std::map<int, std::vector<int>> & node_ids_for_element_id);

  /**
   * Required for converting an MFEMMesh to an MFEMParMesh. This method updates the two-way mappings
   * so that the libMesh global node IDs now correctly map to the LOCAL MFEM dofs for the
   * MFEMParMesh. This method is called internally after creating an MFEMParMesh from an MFEMMesh.
   * NB: failure to call this method will result in the synchronization steps failing.
   */
  void convertSerialDofMappingsToParallel(const MFEMMesh & serial_mesh,
                                          const MFEMParMesh & parallel_mesh);

  /**
   * Add block elements to _block_info.
   */
  void buildCubitBlockInfo(const std::vector<int> & unique_block_ids);

  /**
   * Blocks/subdomains are separate subsets of the mesh that could have different
   * material properties etc. This method returns a vector containing the unique
   * IDs of each block in the mesh. This will be passed to the MFEMMesh constructor
   * which sets the attribute of each element to the ID of the block that it is a
   * part of.
   */
  std::vector<int> getLibmeshBlockIDs() const;

  /**
   * Returns a vector containing the IDs of all boundaries.
   */
  std::vector<int> getSideBoundaryIDs() const;

  /**
   * Maps from the element ID to the block ID.
   */
  std::map<int, int>
  getBlockIDForElementID(const std::map<int, std::vector<int>> & element_ids_for_block_id) const;

  /**
   * Maps from the boundary ID to a vector containing the block IDs of all elements that lie on
   * the boundary.
   */
  std::map<int, std::vector<int>> getBlockIDsForBoundaryID(
      const std::map<int, std::vector<int>> & element_ids_for_block_id,
      const std::map<int, std::vector<int>> & element_ids_for_boundary_id) const;

  /**
   * Returns the libMesh partitioning. The "raw" pointer is wrapped up in a unique
   * pointer.
   */
  std::unique_ptr<int[]> getMeshPartitioning();

  /**
   * Returns true if mesh is split between two or more processors.
   */
  bool isDistributedMesh() const;

  /**
   * Override methods in Exclusive MFEMMesh.
   */
  void buildMFEMMesh() override;
  void buildMFEMParMesh() override;

  /**
   * Returns a constant reference to the block info.
   */
  inline const CubitBlockInfo & blockInfo() const { return _block_info; }

  /**
   * Returns a non-const reference to the block info.
   */
  inline CubitBlockInfo & blockInfo() { return _block_info; }

  /**
   * Returns a const reference to the block element.
   */
  inline const CubitElementInfo & blockElement(int block_id)
  {
    return blockInfo().blockElement(block_id);
  }

private:
  /**
   * Stores the element info for each block in the mesh.
   */
  CubitBlockInfo _block_info;

  /**
   * Maps from a libMesh global node ID to the MFEM LOCAL node ID (dof) on the processor. NB: this
   * is NOT the same as LOCAL TRUE dof.
   */
  std::map<int, int> _libmesh_global_node_id_for_mfem_local_node_id;
  std::map<int, int> _mfem_local_node_id_for_libmesh_global_node_id;
};
