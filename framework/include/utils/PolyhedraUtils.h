//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "libMeshReducedNamespace.h"

#include "libmesh/point.h"

#include <memory>
#include <vector>

namespace PolyhedraUtils
{
  /**
   * Lightweight description of one polyhedral face, using vertex indices.
   *
   * - vertices: indices into the vertex list passed to the splitter
   * - orig_side: mapping back to the original element face index:
   *     -1   : this face lies on the cut interface (inherits cut_face_id)
   *     >= 0 : index into elem_side_list / original side list
   *
   * Faces introduced internally by a split should use orig_side values other
   * than -1 so they do not receive cut_face_id.
   */
  struct FaceDescriptor
  {
    std::vector<unsigned int> vertices;
    int orig_side = -1;
  };

  /**
   * Attempt to split a non-convex polyhedron (given only by vertices and faces)
   * into two or more convex pieces by cutting along one concave region.
   *
   * The caller is responsible for:
   *  - providing the distinct vertex positions via existing_nodes
   *  - passing FaceDescriptor::orig_side for boundary reattachment
   *
   * On success, this routine:
   *  - Adds one or more C0Polyhedron elements to the mesh (children of orig_elem)
   *  - Re-assigns the various boundary ids based on orig_side and cut_face_id
   *  - Sets each new element's subdomain_id() = orig_elem->subdomain_id() + sid_shift_base
   *
   * The routine does *not* mark orig_elem for deletion
   *
   * Return value:
   *  - true  : at least one convex C0Polyhedron was created
   *  - false : could not find a valid split; caller should handle this case
   *
   * This implementation is intentionally conservative: if the geometry is not
   * amenable to a simple convex decomposition, it returns false rather than
   * constructing questionable elements.
   */
  bool splitNonConvexPolyhedron(ReplicatedMesh & mesh,
                                Elem * orig_elem,
                                const std::vector<const Node *> & existing_nodes,
                                const std::vector<FaceDescriptor> & faces,
                                const std::vector<std::vector<boundary_id_type>> & elem_side_list,
                                const subdomain_id_type sid_shift_base,
                                const boundary_id_type cut_face_id);
} // namespace PolyhedraUtils
