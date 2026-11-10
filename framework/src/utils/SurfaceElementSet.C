//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceElementSet.h"
#include "SurfaceEdge2.h"
#include "SurfaceTri3.h"
#include "MooseError.h"

#include "libmesh/string_to_enum.h"

SurfaceElementSet
SurfaceElementSet::fromMesh(const MeshBase & mesh)
{
  SurfaceElementSet set;

  const auto n = mesh.n_active_elem();
  set._elements.reserve(n);
  set._centroids.reserve(n);

  for (const auto * elem : mesh.active_element_ptr_range())
    set.addElement(elem);

  return set;
}

SurfaceElementSet
SurfaceElementSet::fromElements(const std::vector<const Elem *> & elems)
{
  SurfaceElementSet set;

  set._elements.reserve(elems.size());
  set._centroids.reserve(elems.size());

  for (const auto * elem : elems)
    set.addElement(elem);

  return set;
}

void
SurfaceElementSet::addElement(const Elem * elem)
{
  mooseAssert(elem, "Element must not be null");

  // Enforce a homogeneous element family: every element must share the type of
  // the first one added, so a group never mixes 2D EDGE2 and 3D TRI3 faces.
  if (!_elements.empty() && _elements.front()->elem().type() != elem->type())
    mooseError("SurfaceElementSet: mixed element types are not supported (found ",
               libMesh::Utility::enum_to_string(elem->type()),
               " after ",
               libMesh::Utility::enum_to_string(_elements.front()->elem().type()),
               ").");

  std::unique_ptr<SurfaceElement> surface_elem;
  if (elem->type() == EDGE2)
    surface_elem = std::make_unique<SurfaceEdge2>(elem);
  else if (elem->type() == TRI3)
    surface_elem = std::make_unique<SurfaceTri3>(elem);
  else
    mooseError("SurfaceElementSet: unsupported element type ",
               libMesh::Utility::enum_to_string(elem->type()),
               ". Only EDGE2 (2D) and TRI3 (3D) surface elements are supported.");

  // Grow the AABB; initialize it from the first element so an empty union does
  // not leave the box in libMesh's inverted default state.
  const auto bbox = elem->loose_bounding_box();
  if (_elements.empty())
    _bounding_box = bbox;
  else
    _bounding_box.union_with(bbox);

  _centroids.emplace_back(elem->vertex_average());
  _elements.emplace_back(std::move(surface_elem));
}
