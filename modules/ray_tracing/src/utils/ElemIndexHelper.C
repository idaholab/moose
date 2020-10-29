//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElemIndexHelper.h"

ElemIndexHelper::ElemIndexHelper(libMesh::MeshBase & mesh,
                                 const std::string & extra_elem_integer_name)
  : _mesh(mesh), _extra_integer(invalid_uint), _initialized(false)
{
  _extra_integer = mesh.add_elem_integer(extra_elem_integer_name);
}

void
ElemIndexHelper::initialize(const SimpleRange<libMesh::MeshBase::element_iterator> elems)
{
  // First, invalidate the integers for all elements that we know about
  // so that we can tell when getting an index for an elem if said elem
  // was not in the range
  for (Elem * elem : _mesh.element_ptr_range())
    elem->set_extra_integer(_extra_integer, DofObject::invalid_id);

  // Set the index in a contiguous manner for all elements in the range
  dof_id_type next_index = 0;
  for (Elem * elem : elems)
    elem->set_extra_integer(_extra_integer, next_index++);

  // Store the max index so that users can use it to initialize data
  // structures that will use these indices
  _max_index = next_index - 1;

  _initialized = true;
}
