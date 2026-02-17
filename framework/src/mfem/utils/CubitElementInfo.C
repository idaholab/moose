#include "CubitElementInfo.h"
#include "libmesh/enum_elem_type.h"
#include "mfem/fem/fe/fe_base.hpp"

/**
 * CubitFaceInfo
 */
CubitFaceInfo::CubitFaceInfo(CubitFaceType face_type, int basis_type) : _face_type(face_type), _basis_type(basis_type)
{
  buildCubitFaceInfo();
}

void
CubitFaceInfo::buildCubitFaceInfo()
{
  switch (_face_type)
  {
    /**
     * 1D
     */
    case (FACE_POINT1):
    {
      _num_face_nodes = 1;
      _num_face_corner_nodes = 1;
      break;
    }
    /**
     * 2D
     */
    case (FACE_EDGE2):
    {
      _num_face_nodes = 2;
      _num_face_corner_nodes = 2;
      break;
    }
    case (FACE_EDGE3):
    {
      _num_face_nodes = 3;
      _num_face_corner_nodes = 2;
      break;
    }
    /**
     * 3D
     */
    case (FACE_TRI3):
    {
      _num_face_nodes = 3;
      _num_face_corner_nodes = 3;
      break;
    }
    case (FACE_TRI6):
    {
      _num_face_nodes = 6;
      _num_face_corner_nodes = 3;
      break;
    }
    case (FACE_QUAD4):
    {
      _num_face_nodes = 4;
      _num_face_corner_nodes = 4;
      break;
    }
    case (FACE_QUAD8):
    {
      _num_face_nodes = 8;
      _num_face_corner_nodes = 4;
      break;
    }
    case (FACE_QUAD9):
    {
      _num_face_nodes = 9; // Includes center node.
      _num_face_corner_nodes = 4;
      break;
    }
    default:
    {
      mooseError("Unsupported face type '", _face_type, "'.");
      break;
    }
  }
}


const std::map<libMesh::ElemMappingType, int> CubitElementInfo::_libmesh_to_mfem_basis_types{ {libMesh::ElemMappingType::RATIONAL_BERNSTEIN_MAP, mfem::BasisType::Positive}, {libMesh::ElemMappingType::LAGRANGE_MAP, mfem::BasisType::ClosedUniform}};

/**
 * CubitElementInfo
 */
CubitElementInfo::CubitElementInfo(libMesh::ElemType elem_type, libMesh::ElemMappingType map_type, int dimension) : _basis_type(_libmesh_to_mfem_basis_types.at(map_type))
{
  switch (dimension)
  {
    case 1:
    {
      buildCubit1DElementInfo(elem_type);
      break;
    }
    case 2:
    {
      buildCubit2DElementInfo(elem_type);
      break;
    }
    case 3:
    {
      buildCubit3DElementInfo(elem_type);
      break;
    }
    default:
    {
      mooseError("Unsupported element dimension ", dimension, ".");
      break;
    }
  }
}

void
CubitElementInfo::buildCubit1DElementInfo(libMesh::ElemType elem_type)
{
  _dimension = 1;

  // FIXME: Need to add support for SEG4
  switch (elem_type)
  {
    case libMesh::ElemType::EDGE2:
    {
      _element_type = ELEMENT_SEG2;
      _num_nodes = 2;
      _order = 1;
      _num_corner_nodes = 2;
      _num_faces = 2;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_POINT1, _basis_type)};
      break;
    }
    case libMesh::ElemType::EDGE3:
    {
      _element_type = ELEMENT_SEG3;
      _num_nodes = 3;
      _order = 2;
      _num_corner_nodes = 2;
      _num_faces = 2;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_POINT1, _basis_type)};
      break;
    }
    case libMesh::ElemType::EDGE4:
    {
      _element_type = ELEMENT_SEG4;
      _num_nodes = 4;
      _order = 3;
      _num_corner_nodes = 2;
      _num_faces = 2;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_POINT1, _basis_type)};
      break;
    }
    default:
    {
      mooseError("Unsupported 1D element of type ", elem_type, ".");
      break;
    }
  }
}

void
CubitElementInfo::buildCubit2DElementInfo(libMesh::ElemType elem_type)
{
  _dimension = 2;

  // FIXME: Need to add support for QUAD8, TRI7
  switch (elem_type)
  {
    case libMesh::ElemType::TRI3:
    {
      _element_type = ELEMENT_TRI3;
      _num_nodes = 3;
      _order = 1;
      _num_corner_nodes = 3;
      _num_faces = 3;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_EDGE2, _basis_type)};
      break;
    }
    case libMesh::ElemType::TRI6:
    {
      _element_type = ELEMENT_TRI6;
      _num_nodes = 6;
      _order = 2;
      _num_corner_nodes = 3;
      _num_faces = 3;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_EDGE3, _basis_type)};
      break;
    }
    case libMesh::ElemType::TRI7:
    {
      _element_type = ELEMENT_TRI7;
      _num_nodes = 7;
      _order = 2;
      _num_corner_nodes = 3;
      _num_faces = 3;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_EDGE3, _basis_type)};
      break;
    }
    case libMesh::ElemType::QUAD4:
    {
      _element_type = ELEMENT_QUAD4;
      _num_nodes = 4;
      _order = 1;
      _num_corner_nodes = 4;
      _num_faces = 4;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_EDGE2, _basis_type)};
      break;
    }
    case libMesh::ElemType::QUAD8:
    {
      _element_type = ELEMENT_QUAD8;
      _num_nodes = 8;
      _order = 2;
      _num_corner_nodes = 4;
      _num_faces = 4;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_EDGE3, _basis_type)};
      break;
    }
    case libMesh::ElemType::QUAD9:
    {
      _element_type = ELEMENT_QUAD9;
      _num_nodes = 9;
      _order = 2;
      _num_corner_nodes = 4;
      _num_faces = 4;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_EDGE3, _basis_type)};
      break;
    }
    default:
    {
      mooseError("Unsupported 2D element of type ", elem_type, ".");
      break;
    }
  }
}

void
CubitElementInfo::buildCubit3DElementInfo(libMesh::ElemType elem_type)
{
  // FIXME: number of nodes is not enough to distinguish between prism14 and tet14
  _dimension = 3;

  switch (elem_type)
  {
    case libMesh::ElemType::TET4:
    {
      _element_type = ELEMENT_TET4;
      _num_nodes = 4;
      _order = 1;
      _num_corner_nodes = 4;
      _num_faces = 4;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_TRI3, _basis_type)};
      break;
    }
    case libMesh::ElemType::TET10:
    {
      _element_type = ELEMENT_TET10;
      _num_nodes = 10;
      _order = 2;
      _num_corner_nodes = 4;
      _num_faces = 4;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_TRI6, _basis_type)};
      break;
    }
    case libMesh::ElemType::TET14:
    {
      _element_type = ELEMENT_TET14;
      _num_nodes = 14;
      _order = 4;
      _num_corner_nodes = 4;
      _num_faces = 4;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_TRI7, _basis_type)};
      break;
    }
    case libMesh::ElemType::HEX8:
    {
      _element_type = ELEMENT_HEX8;
      _num_nodes = 8;
      _order = 1;
      _num_corner_nodes = 8;
      _num_faces = 6;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_QUAD4, _basis_type)};
      break;
    }
    case libMesh::ElemType::HEX20:
    {
      _element_type = ELEMENT_HEX20;
      _num_nodes = 20;
      _order = 2;
      _num_corner_nodes = 8;
      _num_faces = 6;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_QUAD8, _basis_type)};
      break;
    }
    case libMesh::ElemType::HEX27:
    {
      _element_type = ELEMENT_HEX27;
      _num_nodes = 27;
      _order = 2;
      _num_corner_nodes = 8;
      _num_faces = 6;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_QUAD9, _basis_type)};
      break;
    }
    case libMesh::ElemType::PRISM6:
    {
      _element_type = ELEMENT_WEDGE6;
      _num_nodes = 6;
      _order = 1;
      _num_corner_nodes = 6;
      _num_faces = 5;
      _face_info = getWedge6FaceInfo();
      break;
    }
    case libMesh::ElemType::PRISM15:
    {
      _element_type = ELEMENT_WEDGE15;
      _num_nodes = 15;
      _order = 2;
      _num_corner_nodes = 6;
      _num_faces = 5;
      _face_info = getWedge15FaceInfo();
      break;
    }
    case libMesh::ElemType::PRISM18:
    {
      _element_type = ELEMENT_WEDGE18;
      _num_nodes = 18;
      _order = 2;
      _num_corner_nodes = 6;
      _num_faces = 5;
      _face_info = getWedge18FaceInfo();
      break;
    }
    case libMesh::ElemType::PYRAMID5:
    {
      _element_type = ELEMENT_PYRAMID5;
      _num_nodes = 5;
      _order = 1;
      _num_corner_nodes = 5;
      _num_faces = 5;
      _face_info = getPyramid5FaceInfo();
      break;
    }
    case libMesh::ElemType::PYRAMID13:
    {
      _element_type = ELEMENT_PYRAMID13;
      _num_nodes = 13;
      _order = 2;
      _num_corner_nodes = 5;
      _num_faces = 5;
      _face_info = getPyramid13FaceInfo();
      _num_corner_nodes = 5;
      break;
    }
    case libMesh::ElemType::PYRAMID14:
    {
      _element_type = ELEMENT_PYRAMID14;
      _num_nodes = 14;
      _order = 2;
      _num_corner_nodes = 5;
      _num_faces = 5;
      _face_info = getPyramid14FaceInfo();
      _num_corner_nodes = 5;
      break;
    }
    default:
    {
      mooseError("Unsupported 3D element of type ", elem_type, ".");
      break;
    }
  }
}

std::vector<CubitFaceInfo>
CubitElementInfo::getWedge6FaceInfo() const
{
  // Refer to "cell_prism.C" line 127.
  // We are using the same side ordering as used in LibMesh.
  CubitFaceInfo tri3 = CubitFaceInfo(CubitFaceInfo::FACE_TRI3, _basis_type);   // Faces 0, 4 (LibMesh)
  CubitFaceInfo quad4 = CubitFaceInfo(CubitFaceInfo::FACE_QUAD4, _basis_type); // Faces 1, 2, 3 (LibMesh)

  return {tri3, quad4, quad4, quad4, tri3};
}

std::vector<CubitFaceInfo>
CubitElementInfo::getWedge15FaceInfo() const
{
  CubitFaceInfo tri6 = CubitFaceInfo(CubitFaceInfo::FACE_TRI6, _basis_type);
  CubitFaceInfo quad8 = CubitFaceInfo(CubitFaceInfo::FACE_QUAD8, _basis_type);

  return {tri6, quad8, quad8, quad8, tri6};
}

std::vector<CubitFaceInfo>
CubitElementInfo::getWedge18FaceInfo() const
{
  CubitFaceInfo tri6 = CubitFaceInfo(CubitFaceInfo::FACE_TRI6, _basis_type);
  CubitFaceInfo quad9 = CubitFaceInfo(CubitFaceInfo::FACE_QUAD9, _basis_type);

  return {tri6, quad9, quad9, quad9, tri6};
}

std::vector<CubitFaceInfo>
CubitElementInfo::getPyramid5FaceInfo() const
{
  // Refer to "cell_pyramid5.C" line 134.
  // We are using the same side ordering as used in LibMesh.
  CubitFaceInfo tri3 = CubitFaceInfo(CubitFaceInfo::FACE_TRI3, _basis_type);
  CubitFaceInfo quad4 = CubitFaceInfo(CubitFaceInfo::FACE_QUAD4, _basis_type);

  return {tri3, tri3, tri3, tri3, quad4};
}

std::vector<CubitFaceInfo>
CubitElementInfo::getPyramid13FaceInfo() const
{
  // Refer to "cell_pyramid13.h"
  // Define Pyramid14: Quad8 base and 4 x Tri6.
  CubitFaceInfo tri6 = CubitFaceInfo(CubitFaceInfo::FACE_TRI6, _basis_type);
  CubitFaceInfo quad8 = CubitFaceInfo(CubitFaceInfo::FACE_QUAD8, _basis_type);

  // Use same ordering as LibMesh ("cell_pyramid13.c"; line 40)
  // front, right, back, left, base (different in MFEM!).
  return {tri6, tri6, tri6, tri6, quad8};
}

std::vector<CubitFaceInfo>
CubitElementInfo::getPyramid14FaceInfo() const
{
  // Refer to "cell_pyramid14.h"
  // Define Pyramid14: Quad9 base and 4 x Tri6.
  CubitFaceInfo tri6 = CubitFaceInfo(CubitFaceInfo::FACE_TRI6, _basis_type);
  CubitFaceInfo quad9 = CubitFaceInfo(CubitFaceInfo::FACE_QUAD9, _basis_type);

  // Use same ordering as LibMesh ("cell_pyramid14.c"; line 44)
  // front, right, back, left, base (different in MFEM!).
  return {tri6, tri6, tri6, tri6, quad9};
}

const CubitFaceInfo &
CubitElementInfo::face(int iface) const
{
  /**
   * Check _face_info initialized.
   */
  if (_face_info.empty())
  {
    mooseError("_face_info is empty.");
  }

  /**
   * Check valid face index.
   */
  bool is_valid_face_index = (iface >= 0 && iface < _num_faces);
  if (!is_valid_face_index)
  {
    mooseError("Face index '", iface, "' is invalid.");
  }

  /**
   * Case 1: single face type --> only store a single face.
   * Case 2: multiple face types --> store each face. Return face at index.
   */
  bool is_single_face_type = (_face_info.size() == 1);

  /**
   * Check vector size matches _num_Faces for multiple face types.
   */
  if (!is_single_face_type && _face_info.size() != _num_faces)
  {
    mooseError("_face_info.size() != _num_faces.");
  }

  return is_single_face_type ? _face_info.front() : _face_info[iface];
};

/**
 * CubitBlockInfo
 */
CubitBlockInfo::CubitBlockInfo(int dimension)
{
  if (!validDimension(dimension))
  {
    mooseError("Invalid dimension '", dimension, "' specified.");
  }

  _dimension = dimension;

  clearBlockElements();
}

void
CubitBlockInfo::addBlockElement(int block_id, libMesh::ElemType elem_type, libMesh::ElemMappingType map_type)
{
  if (hasBlockID(block_id))
    mooseError("Block with ID '", block_id, "' has already been added.");
  else if (!validBlockID(block_id))
    mooseError("Illegal block ID '", block_id, "'.");

  auto block_element = CubitElementInfo(elem_type, map_type, _dimension);

  /**
   * Check element is compatible with existing element blocks.
   */
  checkElementBlockIsCompatible(block_element);

  if (!hasBlocks()) // Set order of elements.
  {
    _order = block_element.order();
    _basis_type = block_element.basisType();
  }

  _block_ids.insert(block_id);
  _block_element_for_block_id[block_id] = block_element;
}

uint8_t
CubitBlockInfo::order() const
{
  if (!hasBlocks())
  {
    mooseError("No elements have been added.");
  }

  return _order;
}

int
CubitBlockInfo::basisType() const
{
  if (!hasBlocks())
  {
    mooseError("No elements have been added.");
  }

  return _basis_type;
}

void
CubitBlockInfo::clearBlockElements()
{
  _order = 0;
  _block_ids.clear();
  _block_element_for_block_id.clear();
  _basis_type = mfem::BasisType::Invalid;
}

bool
CubitBlockInfo::hasBlockID(int block_id) const
{
  return (_block_ids.count(block_id) > 0);
}

bool
CubitBlockInfo::validBlockID(int block_id) const
{
  return (block_id > 0); // 1-based indexing.
}

bool
CubitBlockInfo::validDimension(int dimension) const
{
  return (dimension > 0 && dimension <= 3);
}

const CubitElementInfo &
CubitBlockInfo::blockElement(int block_id) const
{
  if (!hasBlockID(block_id))
  {
    mooseError("No element info for block ID '", block_id, "'.");
  }

  return _block_element_for_block_id.at(block_id);
}

void
CubitBlockInfo::checkElementBlockIsCompatible(const CubitElementInfo & new_block_element) const
{
  if (!hasBlocks())
  {
    return;
  }

  // Enforce block orders to be the same for now.
  if (order() != new_block_element.order())
  {
    mooseError("All block elements must be of the same order.");
  }
}
