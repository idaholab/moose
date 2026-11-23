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

#include "MooseError.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/id_types.h"
#include "libmesh/utility.h"
#include "mfem/config/config.hpp"
#include "mfem/fem/fe/fe_base.hpp"

#include <cstdint>
#include <map>
#include <set>
#include <vector>

/**
 * LibmeshMFEMBlockInfo
 *
 * Stores the information about each block in a mesh. Each block can contain a different
 * element type (although all element types must be of the same order and dimension).
 */
class LibmeshMFEMBlockInfo
{
public:
  /**
   * Construct the block information for a mesh of dimension \p dimension. \p fallback indicates
   * whether element types MFEM can not represent should be replaced by the closest type it can,
   * and \p force_first_order whether every element should be represented as first order.
   */
  LibmeshMFEMBlockInfo(int dimension, bool fallback, bool force_first_order);

  /**
   * How a libMesh element type is represented in MFEM.
   */
  struct ElementInfo
  {
    /// The MFEM element type
    mfem::Element::Type mfem_elem_type;
    /// The dimension of the element
    int dimension;
    /// The number of libMesh nodes in the element
    int num_nodes;
    /// The number of nodes at the corners of the element, which become MFEM vertices
    int num_corner_nodes;
    /// The number of sides of the element
    int num_faces;
    /// The polynomial order of the element geometry
    int order;
    /// The libMesh element type of each side, indexed by libMesh side number
    std::vector<libMesh::ElemType> faces;
    /// For each MFEM node of the element, the 1-based index of the corresponding libMesh node
    std::vector<int> mfem_to_libmesh;
    /// For each MFEM node that has no corresponding libMesh node, the weights to give each
    /// libMesh node of the element (by libMesh local index) when interpolating its position
    std::vector<std::vector<mfem::real_t>> additional_points;
  };

  /**
   * Returns a constant reference to the element info for a particular block.
   */
  const ElementInfo & blockElement(libMesh::subdomain_id_type block_id) const;

  /**
   * Returns information for a particular face in a particular block.
   */
  const ElementInfo & blockFace(libMesh::subdomain_id_type block_id, unsigned int face_id) const;

  /**
   * Call to add each block individually.
   */
  void addBlockElement(libMesh::subdomain_id_type block_id,
                       libMesh::ElemType elem_type,
                       libMesh::ElemMappingType map_type);

  /**
   * Accessors.
   */
  uint8_t order() const;
  inline uint8_t dimension() const { return _dimension; }
  int basisType() const;

  inline bool hasBlocks() const { return !blockIDs().empty(); }

private:
  /**
   * Reset all block elements. Called internally in initializer.
   */
  void clearBlockElements();

  /**
   * Helper methods.
   */
  inline const std::set<libMesh::subdomain_id_type> & blockIDs() const { return _block_ids; }

  bool hasBlockID(libMesh::subdomain_id_type block_id) const;
  bool validBlockID(libMesh::subdomain_id_type block_id) const;
  bool validDimension(int dimension) const;

  const ElementInfo & getElementInfo(libMesh::ElemType elem_type, bool warn = false) const;

  /**
   * Stores all block IDs.
   */
  std::set<libMesh::subdomain_id_type> _block_ids;

  const bool _fallback;
  const bool _force_first_order;

  /**
   * Maps from block ID to element.
   */
  std::map<libMesh::subdomain_id_type, libMesh::ElemType> _block_element_for_block_id;

  /**
   * Dimension and order of block elements.
   */
  const uint8_t _dimension;
  uint8_t _order;
  int _basis_type;

  /// Map between libMesh element types and information about that element.
  static const std::map<libMesh::ElemType, ElementInfo> _elem_info;

  /// Map between libMesh element types not supported by MFEM and
  /// simpler types which can approximate them.
  static const std::map<libMesh::ElemType, libMesh::ElemType> _fallback_types;

  /// Map between libMesh element types and the first-order version of that shape.
  static const std::map<libMesh::ElemType, libMesh::ElemType> _first_order_types;

  /**
   * Map from enums for basis types used for higher-order elements in libmesh and MFEM.
   */
  static const std::map<libMesh::ElemMappingType, int> _libmesh_to_mfem_basis_types;
};

#endif
