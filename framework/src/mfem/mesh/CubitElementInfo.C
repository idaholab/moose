#include "CubitElementInfo.h"

/**
 * CubitFaceInfo
 */
CubitFaceInfo::CubitFaceInfo(CubitFaceType face_type) : _face_type(face_type)
{
  buildCubitFaceInfo();
}

void
CubitFaceInfo::buildCubitFaceInfo()
{
  switch (_face_type)
  {
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

/**
 * CubitElementInfo
 */
CubitElementInfo::CubitElementInfo(int num_nodes_per_element, int dimension)
{
  switch (dimension)
  {
    case 2:
    {
      buildCubit2DElementInfo(num_nodes_per_element);
      break;
    }
    case 3:
    {
      buildCubit3DElementInfo(num_nodes_per_element);
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
CubitElementInfo::buildCubit2DElementInfo(int num_nodes_per_element)
{
  _dimension = 2;
  _num_nodes = num_nodes_per_element;

  switch (num_nodes_per_element)
  {
    case 3:
    {
      _element_type = ELEMENT_TRI3;
      _order = 1;
      _num_corner_nodes = 3;
      _num_faces = 3;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_EDGE2)};
      break;
    }
    case 6:
    {
      _element_type = ELEMENT_TRI6;
      _order = 2;
      _num_corner_nodes = 3;
      _num_faces = 3;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_EDGE3)};
      break;
    }
    case 4:
    {
      _element_type = ELEMENT_QUAD4;
      _order = 1;
      _num_corner_nodes = 4;
      _num_faces = 4;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_EDGE2)};
      break;
    }
    case 9:
    {
      _element_type = ELEMENT_QUAD9;
      _order = 2;
      _num_corner_nodes = 4;
      _num_faces = 4;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_EDGE3)};
      break;
    }
    default:
    {
      mooseError("Unsupported 2D element with ", num_nodes_per_element, " nodes per element.");
      break;
    }
  }
}

void
CubitElementInfo::buildCubit3DElementInfo(int num_nodes_per_element)
{
  _dimension = 3;
  _num_nodes = num_nodes_per_element;

  switch (num_nodes_per_element)
  {
    case 4:
    {
      _element_type = ELEMENT_TET4;
      _order = 1;
      _num_corner_nodes = 4;
      _num_faces = 4;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_TRI3)};
      break;
    }
    case 10:
    {
      _element_type = ELEMENT_TET10;
      _order = 2;
      _num_corner_nodes = 4;
      _num_faces = 4;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_TRI6)};
      break;
    }
    case 8:
    {
      _element_type = ELEMENT_HEX8;
      _order = 1;
      _num_corner_nodes = 8;
      _num_faces = 6;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_QUAD4)};
      break;
    }
    case 27:
    {
      _element_type = ELEMENT_HEX27;
      _order = 2;
      _num_corner_nodes = 8;
      _num_faces = 6;
      _face_info = {CubitFaceInfo(CubitFaceInfo::FACE_QUAD9)};
      break;
    }
    case 6:
    {
      _element_type = ELEMENT_WEDGE6;
      _order = 1;
      _num_corner_nodes = 6;
      _num_faces = 5;
      _face_info = getWedge6FaceInfo();
      break;
    }
    case 18:
    {
      _element_type = ELEMENT_WEDGE18;
      _order = 2;
      _num_corner_nodes = 6;
      _num_faces = 5;
      _face_info = getWedge18FaceInfo();
      break;
    }
    case 5:
    {
      _element_type = ELEMENT_PYRAMID5;
      _order = 1;
      _num_corner_nodes = 5;
      _num_faces = 5;
      _face_info = getPyramid5FaceInfo();
      break;
    }
    case 14:
    {
      _element_type = ELEMENT_PYRAMID14;
      _order = 2;
      _num_corner_nodes = 5;
      _num_faces = 5;
      _face_info = getPyramid14FaceInfo();
      _num_corner_nodes = 5;
      break;
    }
    default:
    {
      mooseError("Unsupported 3D element with ", num_nodes_per_element, " nodes per element.");
      break;
    }
  }
}

std::vector<CubitFaceInfo>
CubitElementInfo::getWedge6FaceInfo() const
{
  // Refer to "cell_prism.C" line 127.
  // We are using the same side ordering as used in LibMesh.
  CubitFaceInfo tri3 = CubitFaceInfo(CubitFaceInfo::FACE_TRI3);   // Faces 0, 4 (LibMesh)
  CubitFaceInfo quad4 = CubitFaceInfo(CubitFaceInfo::FACE_QUAD4); // Faces 1, 2, 3 (LibMesh)

  return {tri3, quad4, quad4, quad4, tri3};
}

std::vector<CubitFaceInfo>
CubitElementInfo::getWedge18FaceInfo() const
{
  CubitFaceInfo tri6 = CubitFaceInfo(CubitFaceInfo::FACE_TRI6);
  CubitFaceInfo quad9 = CubitFaceInfo(CubitFaceInfo::FACE_QUAD9);

  return {tri6, quad9, quad9, quad9, tri6};
}

std::vector<CubitFaceInfo>
CubitElementInfo::getPyramid5FaceInfo() const
{
  // Refer to "cell_pyramid5.C" line 134.
  // We are using the same side ordering as used in LibMesh.
  CubitFaceInfo tri3 = CubitFaceInfo(CubitFaceInfo::FACE_TRI3);
  CubitFaceInfo quad4 = CubitFaceInfo(CubitFaceInfo::FACE_QUAD4);

  return {tri3, tri3, tri3, tri3, quad4};
}

std::vector<CubitFaceInfo>
CubitElementInfo::getPyramid14FaceInfo() const
{
  // Refer to "cell_pyramid14.h"
  // Define Pyramid14: Quad9 base and 4 x Tri6.
  CubitFaceInfo tri6 = CubitFaceInfo(CubitFaceInfo::FACE_TRI6);
  CubitFaceInfo quad9 = CubitFaceInfo(CubitFaceInfo::FACE_QUAD9);

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
CubitBlockInfo::addBlockElement(int block_id, int num_nodes_per_element)
{
  if (hasBlockID(block_id))
    mooseError("Block with ID '", block_id, "' has already been added.");
  else if (!validBlockID(block_id))
    mooseError("Illegal block ID '", block_id, "'.");

  auto block_element = CubitElementInfo(num_nodes_per_element, _dimension);

  /**
   * Check element is compatible with existing element blocks.
   */
  checkElementBlockIsCompatible(block_element);

  if (!hasBlocks()) // Set order of elements.
  {
    _order = block_element.order();
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

void
CubitBlockInfo::clearBlockElements()
{
  _order = 0;
  _block_ids.clear();
  _block_element_for_block_id.clear();
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
  return (dimension == 2 || dimension == 3);
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
