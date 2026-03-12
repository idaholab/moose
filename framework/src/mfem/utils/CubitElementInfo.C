#include "CubitElementInfo.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_order.h"
#include "mfem/fem/fe/fe_base.hpp"

namespace
{
const std::vector<std::vector<mfem::real_t>> no_additional_points(0);
};

const std::map<libMesh::ElemType, CubitBlockInfo::ElementInfo> CubitBlockInfo::_elem_info{
    {libMesh::ElemType::NODEELEM,
     {mfem::Element::Type::POINT, 0, 1, 1, 0, 0, {}, {1}, ::no_additional_points}},
    {libMesh::ElemType::EDGE2,
     {mfem::Element::Type::SEGMENT,
      1,
      2,
      2,
      2,
      1,
      {libMesh::ElemType::NODEELEM, libMesh::ElemType::NODEELEM},
      {1, 2},
      ::no_additional_points}},
    {libMesh::ElemType::EDGE3,
     {mfem::Element::Type::SEGMENT,
      1,
      3,
      2,
      2,
      2,
      {libMesh::ElemType::NODEELEM, libMesh::ElemType::NODEELEM},
      {1, 2, 3},
      ::no_additional_points}},
    {libMesh::ElemType::EDGE4,
     {mfem::Element::Type::SEGMENT,
      1,
      4,
      2,
      2,
      3,
      {libMesh::ElemType::NODEELEM, libMesh::ElemType::NODEELEM},
      {1, 2, 3, 4},
      ::no_additional_points}},
    {libMesh::ElemType::TRI3,
     {mfem::Element::Type::TRIANGLE,
      2,
      3,
      3,
      3,
      1,
      {libMesh::ElemType::EDGE2, libMesh::ElemType::EDGE2, libMesh::ElemType::EDGE2},
      {1, 2, 3},
      ::no_additional_points}},
    {libMesh::ElemType::TRI6,
     {mfem::Element::Type::TRIANGLE,
      2,
      6,
      3,
      3,
      2,
      {libMesh::ElemType::EDGE3, libMesh::ElemType::EDGE3, libMesh::ElemType::EDGE3},
      {1, 2, 3, 4, 5, 6},
      ::no_additional_points}},
    // {libMesh::ElemType::TRI7,
    //  {mfem::Element::Type::TRIANGLE,
    //   2,
    //   7,
    //   3,
    //   3,
    //   3,
    //   {libMesh::ElemType::EDGE3, libMesh::ElemType::EDGE3, libMesh::ElemType::EDGE3},
    //   {1, 2, 3, 4, 5, 6, 7},
    //   ::no_additional_points}},
    {libMesh::ElemType::QUAD4,
     {mfem::Element::Type::QUADRILATERAL,
      2,
      4,
      4,
      4,
      1,
      {libMesh::ElemType::EDGE2,
       libMesh::ElemType::EDGE2,
       libMesh::ElemType::EDGE2,
       libMesh::ElemType::EDGE2},
      {1, 2, 3, 4},
      ::no_additional_points}},
    {libMesh::ElemType::QUAD8,
     {mfem::Element::Type::QUADRILATERAL,
      2,
      8,
      4,
      4,
      2,
      {libMesh::ElemType::EDGE3,
       libMesh::ElemType::EDGE3,
       libMesh::ElemType::EDGE3,
       libMesh::ElemType::EDGE3},
      {1, 2, 3, 4, 5, 6, 7, 8},
      {{-.25, -.25, -.25, -.25, .5, .5, .5, .5}}}},
    {libMesh::ElemType::QUAD9,
     {mfem::Element::Type::QUADRILATERAL,
      2,
      9,
      4,
      4,
      2,
      {libMesh::ElemType::EDGE3,
       libMesh::ElemType::EDGE3,
       libMesh::ElemType::EDGE3,
       libMesh::ElemType::EDGE3},
      {1, 2, 3, 4, 5, 6, 7, 8, 9},
      ::no_additional_points}},
    {libMesh::ElemType::TET4,
     {mfem::Element::Type::TETRAHEDRON,
      3,
      4,
      4,
      4,
      1,
      {libMesh::ElemType::TRI3,
       libMesh::ElemType::TRI3,
       libMesh::ElemType::TRI3,
       libMesh::ElemType::TRI3},
      {1, 2, 3, 4},
      ::no_additional_points}},
    {libMesh::ElemType::TET10,
     {mfem::Element::Type::TETRAHEDRON,
      3,
      10,
      4,
      4,
      2,
      {libMesh::ElemType::TRI6,
       libMesh::ElemType::TRI6,
       libMesh::ElemType::TRI6,
       libMesh::ElemType::TRI6},
      {1, 2, 3, 4, 5, 7, 8, 6, 9, 10},
      ::no_additional_points}},
    // {libMesh::ElemType::TET14,
    //  {mfem::Element::Type::TETRAHEDRON,
    //   3,
    //   14,
    //   4,
    //   4,
    //   3,
    //   {libMesh::ElemType::TRI7,
    //    libMesh::ElemType::TRI7,
    //    libMesh::ElemType::TRI7,
    //    libMesh::ElemType::TRI7},
    //   {1, 2, 3, 4, 5, 7, 8, 6, 9, 10, 11, 12, 13, 14},
    //   ::no_additional_points}},
    {libMesh::ElemType::HEX8,
     {mfem::Element::Type::HEXAHEDRON,
      3,
      8,
      8,
      6,
      1,
      {libMesh::ElemType::QUAD4,
       libMesh::ElemType::QUAD4,
       libMesh::ElemType::QUAD4,
       libMesh::ElemType::QUAD4,
       libMesh::ElemType::QUAD4,
       libMesh::ElemType::QUAD4},
      {1, 2, 3, 4, 5, 6, 7, 8},
      ::no_additional_points}},
    {libMesh::ElemType::HEX20,
     {mfem::Element::Type::HEXAHEDRON,
      3,
      20,
      8,
      6,
      2,
      {libMesh::ElemType::QUAD8,
       libMesh::ElemType::QUAD8,
       libMesh::ElemType::QUAD8,
       libMesh::ElemType::QUAD8,
       libMesh::ElemType::QUAD8,
       libMesh::ElemType::QUAD8},
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 17, 18, 19, 20, 13, 14, 15, 16},
      {
          {-.25, -.25, -.25, -.25, 0., 0., 0., 0., .5, .5, .5, .5, 0., 0., 0., 0., 0., 0., 0., 0.},
          {-.25, -.25, 0.,  0.,  -.25, -.25, 0.,  0., .5, 0.,
           0.,   0.,   0.5, 0.5, 0.,   0.,   0.5, 0., 0., 0.},
          {0., -.25, -.25, 0.,  0.,  -.25, -.25, 0.,  0., 0.5,
           0., 0.,   0.,   0.5, 0.5, 0.,   0.,   0.5, 0., 0.},
          {0.,  0., -.25, -.25, 0.,  0.,  -.25, -.25, 0.,  0.,
           0.5, 0., 0.,   0.,   0.5, 0.5, 0.,   0.,   0.5, 0.},
          {-.25, 0.,  0.,  -.25, -.25, 0.,  0., -.25, 0., 0.,
           0.,   0.5, 0.5, 0.,   0.,   0.5, 0., 0.,   0., 0.5},
          {0., 0., 0., 0., -.25, -.25, -.25, -.25, 0., 0., 0., 0., 0., 0., 0., 0., .5, .5, .5, .5},
          {-.25, -.25, -.25, -.25, -.25, -.25, -.25, -.25, 0.25, 0.25,
           0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25},
      }}},
    {libMesh::ElemType::HEX27,
     {mfem::Element::Type::HEXAHEDRON,
      3,
      27,
      8,
      6,
      2,
      {libMesh::ElemType::QUAD9,
       libMesh::ElemType::QUAD9,
       libMesh::ElemType::QUAD9,
       libMesh::ElemType::QUAD9,
       libMesh::ElemType::QUAD9,
       libMesh::ElemType::QUAD9},
      {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 17, 18,
       19, 20, 13, 14, 15, 16, 21, 22, 23, 24, 25, 26, 27},
      ::no_additional_points}},
    {libMesh::ElemType::PRISM6,
     {mfem::Element::Type::WEDGE,
      3,
      6,
      6,
      5,
      1,
      {libMesh::ElemType::TRI3,
       libMesh::ElemType::QUAD4,
       libMesh::ElemType::QUAD4,
       libMesh::ElemType::QUAD4,
       libMesh::ElemType::TRI3},
      {1, 2, 3, 4, 5, 6},
      ::no_additional_points}},
    {libMesh::ElemType::PRISM15,
     {mfem::Element::Type::WEDGE,
      3,
      15,
      6,
      5,
      2,
      {libMesh::ElemType::TRI6,
       libMesh::ElemType::QUAD8,
       libMesh::ElemType::QUAD8,
       libMesh::ElemType::QUAD8,
       libMesh::ElemType::TRI6},
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 13, 14, 15, 10, 11, 12},
      {{-.25, -.25, 0., -.25, -.25, 0., 0.5, 0., 0., 0.5, 0.5, 0., 0.5, 0., 0.},
       {0., -.25, -.25, 0., -.25, -.25, 0., 0.5, 0., 0., 0.5, 0.5, 0., 0.5, 0.},
       {-.25, 0., -.25, -.25, 0., -.25, 0., 0., 0.5, 0.5, 0., 0.5, 0., 0., 0.5}}}},
    {libMesh::ElemType::PRISM18,
     {mfem::Element::Type::WEDGE,
      3,
      18,
      6,
      5,
      2,
      {libMesh::ElemType::TRI6,
       libMesh::ElemType::QUAD9,
       libMesh::ElemType::QUAD9,
       libMesh::ElemType::QUAD9,
       libMesh::ElemType::TRI6},
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 13, 14, 15, 10, 11, 12, 16, 17, 18},
      ::no_additional_points}},
    {libMesh::ElemType::PYRAMID5,
     {mfem::Element::Type::PYRAMID,
      3,
      5,
      5,
      5,
      1,
      {libMesh::ElemType::TRI3,
       libMesh::ElemType::TRI3,
       libMesh::ElemType::TRI3,
       libMesh::ElemType::TRI3,
       libMesh::ElemType::QUAD4},
      {1, 2, 3, 4, 5},
      ::no_additional_points}},
    {libMesh::ElemType::PYRAMID13,
     {mfem::Element::Type::PYRAMID,
      3,
      13,
      5,
      5,
      2,
      {libMesh::ElemType::TRI6,
       libMesh::ElemType::TRI6,
       libMesh::ElemType::TRI6,
       libMesh::ElemType::TRI6,
       libMesh::ElemType::QUAD8},
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},
      {{-0.25, -0.25, -0.25, -0.25, 0., 0.5, 0.5, 0.5, 0.5, 0., 0., 0., 0.}}}},
    {libMesh::ElemType::PYRAMID14,
     {mfem::Element::Type::PYRAMID,
      3,
      14,
      5,
      5,
      2,
      {libMesh::ElemType::TRI6,
       libMesh::ElemType::TRI6,
       libMesh::ElemType::TRI6,
       libMesh::ElemType::TRI6,
       libMesh::ElemType::QUAD9},
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14},
      ::no_additional_points}}};

const std::map<libMesh::ElemType, libMesh::ElemType> CubitBlockInfo::_first_order_types{
    {libMesh::ElemType::NODEELEM, libMesh::ElemType::NODEELEM},
    {libMesh::ElemType::EDGE2, libMesh::ElemType::EDGE2},
    {libMesh::ElemType::EDGE3, libMesh::ElemType::EDGE2},
    {libMesh::ElemType::EDGE4, libMesh::ElemType::EDGE2},
    {libMesh::ElemType::TRI3, libMesh::ElemType::TRI3},
    {libMesh::ElemType::TRI6, libMesh::ElemType::TRI3},
    {libMesh::ElemType::TRI7, libMesh::ElemType::TRI3},
    {libMesh::ElemType::QUAD4, libMesh::ElemType::QUAD4},
    {libMesh::ElemType::QUAD8, libMesh::ElemType::QUAD4},
    {libMesh::ElemType::QUAD9, libMesh::ElemType::QUAD4},
    {libMesh::ElemType::TET4, libMesh::ElemType::TET4},
    {libMesh::ElemType::TET10, libMesh::ElemType::TET4},
    {libMesh::ElemType::TET14, libMesh::ElemType::TET4},
    {libMesh::ElemType::HEX8, libMesh::ElemType::HEX8},
    {libMesh::ElemType::HEX20, libMesh::ElemType::HEX8},
    {libMesh::ElemType::HEX27, libMesh::ElemType::HEX8},
    {libMesh::ElemType::PRISM6, libMesh::ElemType::PRISM6},
    {libMesh::ElemType::PRISM15, libMesh::ElemType::PRISM6},
    {libMesh::ElemType::PRISM18, libMesh::ElemType::PRISM6},
    {libMesh::ElemType::PRISM20, libMesh::ElemType::PRISM6},
    {libMesh::ElemType::PRISM21, libMesh::ElemType::PRISM6},
    {libMesh::ElemType::PYRAMID5, libMesh::ElemType::PYRAMID5},
    {libMesh::ElemType::PYRAMID13, libMesh::ElemType::PYRAMID5},
    {libMesh::ElemType::PYRAMID14, libMesh::ElemType::PYRAMID5},
    {libMesh::ElemType::PYRAMID18, libMesh::ElemType::PYRAMID5},
};

const std::map<libMesh::ElemType, libMesh::ElemType> CubitBlockInfo::_fallback_types{
    {libMesh::ElemType::TRI7, libMesh::ElemType::TRI6},
    {libMesh::ElemType::TET14, libMesh::ElemType::TET10},
    {libMesh::ElemType::PRISM20, libMesh::ElemType::PRISM18},
    {libMesh::ElemType::PRISM21, libMesh::ElemType::PRISM18},
    {libMesh::ElemType::PYRAMID18, libMesh::ElemType::PYRAMID14},
};

const std::map<libMesh::ElemMappingType, int> CubitBlockInfo::_libmesh_to_mfem_basis_types{
    {libMesh::ElemMappingType::RATIONAL_BERNSTEIN_MAP, mfem::BasisType::Positive},
    {libMesh::ElemMappingType::LAGRANGE_MAP, mfem::BasisType::ClosedUniform}};

const CubitBlockInfo::ElementInfo &
CubitBlockInfo::getElementInfo(libMesh::ElemType elem_type, bool warn) const
{
  if (_force_first_order)
  {
    elem_type = _first_order_types.at(elem_type);
  }
  if (elem_type == libMesh::ElemType::PYRAMID13 || elem_type == libMesh::ElemType::PYRAMID14 ||
      elem_type == libMesh::ElemType::PYRAMID18)
  {
    mooseError("Due to bug in MFEM, can not convert higher order libMesh pyramid elements.");
  }
  auto search = CubitBlockInfo::_elem_info.find(elem_type);
  if (search != CubitBlockInfo::_elem_info.end())
  {
    return search->second;
  }
  else if (_fallback)
  {
    auto search = CubitBlockInfo::_fallback_types.find(elem_type);
    if (search != CubitBlockInfo::_fallback_types.end())
    {
      if (warn)
      {
        mooseWarning("Can not represent libMesh element type ",
                     elem_type,
                     " in MFEM mesh. Falling back to use element type ",
                     search->second,
                     ".");
      }
      return getElementInfo(search->second);
    }
  }
  mooseError("Can not represent libMesh element type ", elem_type, " in MFEM mesh.");
}

/**
 * CubitBlockInfo
 */
CubitBlockInfo::CubitBlockInfo(int dimension, bool fallback, bool force_first_order)
{
  if (!validDimension(dimension))
  {
    mooseError("Invalid dimension '", dimension, "' specified.");
  }

  _dimension = dimension;
  _fallback = fallback;
  _force_first_order = force_first_order;

  clearBlockElements();
}

void
CubitBlockInfo::addBlockElement(int block_id,
                                libMesh::ElemType elem_type,
                                libMesh::ElemMappingType map_type)
{
  if (hasBlockID(block_id))
    mooseError("Block with ID '", block_id, "' has already been added.");
  else if (!validBlockID(block_id))
    mooseError("Illegal block ID '", block_id, "'.");

  const auto & block_element = getElementInfo(elem_type, true);

  if (!hasBlocks()) // Set order of elements.
  {
    _order = block_element.order;
    _basis_type = _libmesh_to_mfem_basis_types.at(map_type);
  }
  else
  {
    /**
     * Check element is compatible with existing element blocks.
     */
    if (_dimension != block_element.dimension)
    {
      mooseError("Element of type ", elem_type, " is not ", _dimension, "D.");
    }

    if (_basis_type != _libmesh_to_mfem_basis_types.at(map_type))
    {
      mooseError("All block elements must have the same mapping type.");
    }

    // Enforce block orders to be the same for now.
    if (_order != block_element.order)
    {
      mooseError("All block elements must be of the same order.");
    }
  }

  _block_ids.insert(block_id);
  _block_element_for_block_id[block_id] = elem_type;
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

const CubitBlockInfo::ElementInfo &
CubitBlockInfo::blockElement(int block_id) const
{
  if (!hasBlockID(block_id))
  {
    mooseError("No element info for block ID '", block_id, "'.");
  }

  return getElementInfo(_block_element_for_block_id.at(block_id));
}

const CubitBlockInfo::ElementInfo &
CubitBlockInfo::blockFace(int block_id, int face_id) const
{
  return getElementInfo(blockElement(block_id).faces[face_id]);
}
