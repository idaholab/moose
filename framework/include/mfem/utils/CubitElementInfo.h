#pragma once
#include <stdint.h>
#include "MooseError.h"
#include "libmesh/enum_elem_type.h"
#include "mfem/fem/fe/fe_base.hpp"

// FIXME: Alex's work on this in a branch put these in namespaces. Should I do that here?

/**
 * CubitFaceInfo
 *
 * Stores information about a particular element face.
 */
class CubitFaceInfo
{
public:
  CubitFaceInfo() = delete;
  ~CubitFaceInfo() = default;

  enum CubitFaceType
  {
    FACE_POINT1,
    FACE_EDGE2,
    FACE_EDGE3,
    FACE_TRI3,
    FACE_TRI6, // order = 2.
    FACE_TRI7, // order = 2. Center node.
    FACE_QUAD4,
    FACE_QUAD8, // order = 2.
    FACE_QUAD9  // order = 2. Center node.
  };

  /**
   * Default initializer.
   */
  CubitFaceInfo(CubitFaceType face_type, int basis_type);

  inline uint8_t order() const { return _order; };
  inline uint8_t numFaceNodes() const { return _num_face_nodes; };
  inline uint8_t numFaceCornerNodes() const { return _num_face_corner_nodes; };
  inline CubitFaceType faceType() const { return _face_type; }
  inline int basisType() const { return _basis_type; }

protected:
  void buildCubitFaceInfo();

private:
  /**
   * Type of face.
   */
  CubitFaceType _face_type;

  /**
   * Total number of nodes and number of corner nodes ("vertices").
   */
  uint8_t _num_face_nodes;
  uint8_t _num_face_corner_nodes;

  /**
   * Order of face.
   */
  uint8_t _order;
  int _basis_type;
};

/**
 * CubitElementInfo
 *
 * Stores information about a particular element.
 */
class CubitElementInfo
{
public:
  CubitElementInfo() = default;
  ~CubitElementInfo() = default;

  CubitElementInfo(libMesh::ElemType elem_type,
                   libMesh::ElemMappingType map_type,
                   int dimension = 3);

  enum CubitElementType
  {
    ELEMENT_SEG2,
    ELEMENT_SEG3,
    ELEMENT_SEG4,
    ELEMENT_TRI3,
    ELEMENT_TRI6,
    ELEMENT_TRI7,
    ELEMENT_QUAD4,
    ELEMENT_QUAD8,
    ELEMENT_QUAD9,
    ELEMENT_TET4,
    ELEMENT_TET10,
    ELEMENT_TET14,
    ELEMENT_HEX8,
    ELEMENT_HEX20,
    ELEMENT_HEX27,
    ELEMENT_WEDGE6,
    ELEMENT_WEDGE15,
    ELEMENT_WEDGE18,
    ELEMENT_PYRAMID5,
    ELEMENT_PYRAMID13,
    ELEMENT_PYRAMID14
  };

  inline CubitElementType elementType() const { return _element_type; }

  /**
   * Returns info for a particular face.
   */
  const CubitFaceInfo & face(int iface = 0) const;

  inline uint8_t numFaces() const { return _num_faces; }

  inline uint8_t numNodes() const { return _num_nodes; }
  inline uint8_t numCornerNodes() const { return _num_corner_nodes; }

  inline uint8_t order() const { return _order; }
  inline uint8_t dimension() const { return _dimension; }
  inline int basisType() const { return _basis_type; }

protected:
  void buildCubit1DElementInfo(libMesh::ElemType elem_type);
  void buildCubit2DElementInfo(libMesh::ElemType elem_type);
  void buildCubit3DElementInfo(libMesh::ElemType elem_type);

  /**
   * Sets the _face_info vector.
   */
  std::vector<CubitFaceInfo> getWedge6FaceInfo() const;
  std::vector<CubitFaceInfo> getWedge15FaceInfo() const;
  std::vector<CubitFaceInfo> getWedge18FaceInfo() const;

  std::vector<CubitFaceInfo> getPyramid5FaceInfo() const;
  std::vector<CubitFaceInfo> getPyramid13FaceInfo() const;
  std::vector<CubitFaceInfo> getPyramid14FaceInfo() const;

private:
  /**
   * Stores the element type.
   */
  CubitElementType _element_type;

  /**
   * NB: first-order elements have only nodes on the "corners". Second-order have
   * additional nodes between "corner" nodes.
   */
  uint8_t _order;
  uint8_t _dimension;
  int _basis_type;

  /**
   * NB: "corner nodes" refer to MOOSE nodes at the corners of an element. In
   * MFEM this is referred to as "vertices".
   */
  uint8_t _num_nodes;
  uint8_t _num_corner_nodes;

  /**
   * Stores info about the face types.
   */
  uint8_t _num_faces;
  std::vector<CubitFaceInfo> _face_info;

  /**
   * Map from enums for basis types used for higher-order elements in libmesh and MFEM.
   */
  static const std::map<libMesh::ElemMappingType, int> _libmesh_to_mfem_basis_types;
};

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
  CubitBlockInfo(int dimension);

  /**
   * Returns a constant reference to the element info for a particular block.
   */
  const CubitElementInfo & blockElement(int block_id) const;

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
  void checkElementBlockIsCompatible(const CubitElementInfo & new_block_element) const;

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

private:
  /**
   * Stores all block IDs.
   */
  std::set<int> _block_ids;

  /**
   * Maps from block ID to element.
   */
  std::map<int, CubitElementInfo> _block_element_for_block_id;

  /**
   * Dimension and order of block elements.
   */
  uint8_t _dimension;
  uint8_t _order;
  int _basis_type;
};
