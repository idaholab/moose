//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SidePtrHelper.h"

// libMesh includes
#include "libmesh/elem.h"

SidePtrHelper::SidePtrHelper() : _threaded_side_ptr_elems(libMesh::n_threads()) {}

const libMesh::Elem *
SidePtrHelper::sidePtrHelper(const libMesh::Elem * elem, const unsigned int s, const THREAD_ID tid)
{
  auto & elems = _threaded_side_ptr_elems[tid];
  if ((std::size_t)elem->type() >= elems.size())
    elems.resize(elem->type() + 1);
  std::unique_ptr<const libMesh::Elem> & side_elem = elems[elem->type()];
  elem->build_side_ptr(side_elem, s);
  return side_elem.get();
}
