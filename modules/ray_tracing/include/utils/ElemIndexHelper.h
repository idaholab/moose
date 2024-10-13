//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseError.h"

// libMesh includes
#include "libmesh/elem.h"
#include "libmesh/elem_range.h"
#include "libmesh/mesh_base.h"

/**
 * Helper for setting up a contiguous index for a given range of elements that are
 * known by this processor.
 *
 * The contiguous index is useful for indexing into alternate data structures for
 * each element without the need for a map.
 *
 * The index is stored on each elem for quick access.
 */
class ElemIndexHelper
{
public:
  /**
   * Constructor.
   *
   * @param mesh The mesh that contains the elements that are to be index into.
   * @param extra_elem_integer_name A name for the extra element integer that will store the index.
   *
   * Make sure to call initialize() after construction!
   */
  ElemIndexHelper(libMesh::MeshBase & mesh, const std::string & extra_elem_integer_name);

  /**
   * Initializes the indices in a contiguous manner for the given element range
   */
  void initialize(const libMesh::SimpleRange<libMesh::MeshBase::element_iterator> elems);

  /**
   * Whether or not the element \p elem has an index set for it using this object.
   */
  bool hasIndex(const libMesh::Elem * elem) const
  {
    mooseAssert(elem, "Null elem");
    mooseAssert(_initialized, "Not initialized");
    mooseAssert(_mesh.query_elem_ptr(elem->id()), "Not an elem of the mesh");
    return elem->get_extra_integer(_extra_integer) != libMesh::DofObject::invalid_id;
  }

  /**
   * Get the index associated with the element \p elem.
   */
  libMesh::dof_id_type getIndex(const libMesh::Elem * elem) const
  {
    mooseAssert(hasIndex(elem), "Elem not in indexed range");
    return elem->get_extra_integer(_extra_integer);
  }

  /**
   * Gets the maximum index generated using this object.
   *
   * Useful for initializing data structures that will be indexed
   * using the indices provided by this object.
   */
  libMesh::dof_id_type maxIndex() const { return _max_index; }

private:
  // The mesh
  libMesh::MeshBase & _mesh;
  /// The extra elem integer that stores the index
  unsigned int _extra_integer;
  /// Whether or not this object is initialized
  bool _initialized;
  /// The max index generated
  libMesh::dof_id_type _max_index;
};
