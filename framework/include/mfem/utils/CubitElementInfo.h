#pragma once
#include <mfem/mesh/element.hpp>
#include <stdint.h>
#include "MooseError.h"
#include "libmesh/enum_elem_type.h"
#include "mfem/config/config.hpp"
#include "mfem/fem/fe/fe_base.hpp"
#include <vector>

// FIXME: Alex's work on this in a branch put these in namespaces. Should I do that here?

/**
 * CubitBlockInfo
 *
 * Stores the information about each block in a mesh. Each block can contain a different
 * element type (although all element types must be of the same order and dimension).
 */
class CubitBlockInfo
{
public:
  CubitBlockInfo() = delete;
  ~CubitBlockInfo() = default;

  /**
   * Default initializer.
   */
  CubitBlockInfo(int dimension, bool fallback, bool force_first_order);

  struct ElementInfo
  {
    mfem::Element::Type mfem_elem_type;
    int dimension;
    int num_nodes;
    int num_corner_nodes;
    int num_faces;
    int order;
    std::vector<libMesh::ElemType> faces;
    std::vector<int> mfem_to_libmesh;
    /// Weightings used to calculate additional control points needed by MFEM
    std::vector<std::vector<mfem::real_t>> additional_points;
  };

  /**
   * Returns a constant reference to the element info for a particular block.
   */
  const ElementInfo & blockElement(int block_id) const;

  /**
   * Returns information for a particular face in a particular block.
   */
  const ElementInfo & blockFace(int block_id, int bound_id) const;

  /**
   * Call to add each block individually.
   */
  void
  addBlockElement(int block_id, libMesh::ElemType elem_type, libMesh::ElemMappingType map_type);

  /**
   * Accessors.
   */
  uint8_t order() const;
  inline uint8_t dimension() const { return _dimension; }
  int basisType() const;

  inline std::size_t numBlocks() const { return blockIDs().size(); }
  inline bool hasBlocks() const { return !blockIDs().empty(); }

protected:
  /**
   * Checks that the order of a new block element matches the order of existing blocks. Called
   * internally in mehtod "addBlockElement".
   */
  // void checkElementBlockIsCompatible(const ElementInfo & new_block_element) const;

  /**
   * Reset all block elements. Called internally in initializer.
   */
  void clearBlockElements();

  /**
   * Helper methods.
   */
  inline const std::set<int> & blockIDs() const { return _block_ids; }

  bool hasBlockID(int block_id) const;
  bool validBlockID(int block_id) const;
  bool validDimension(int dimension) const;

  const ElementInfo & getElementInfo(libMesh::ElemType elem_type, bool warn = false) const;

private:
  /**
   * Stores all block IDs.
   */
  std::set<int> _block_ids;

  bool _fallback;
  bool _force_first_order;

  /**
   * Maps from block ID to element.
   */
  std::map<int, libMesh::ElemType> _block_element_for_block_id;

  /**
   * Dimension and order of block elements.
   */
  uint8_t _dimension;
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
