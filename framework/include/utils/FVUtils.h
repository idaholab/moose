//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "MooseError.h"
#include "MooseMeshUtils.h"
#include "FaceInfo.h"
#include "libmesh/elem.h"

#include <tuple>

namespace Moose
{
namespace FV
{
template <typename ActionFunctor>
void
loopOverElemFaceInfo(const Elem & elem,
                     const MooseMesh & mesh,
                     ActionFunctor & act,
                     const Moose::CoordinateSystemType coord_type,
                     const unsigned int rz_radial_coord = libMesh::invalid_uint)
{
  mooseAssert(elem.active(), "We should never call this method with an inactive element");

  for (const auto side : elem.side_index_range())
  {
    const Elem * const candidate_neighbor = elem.neighbor_ptr(side);

    bool elem_has_info;

    std::set<const Elem *> neighbors;

    // See MooseMesh::buildFiniteVolumeInfo for corresponding checks/additions of FaceInfo
    if (!candidate_neighbor)
    {
      neighbors.insert(candidate_neighbor);
      elem_has_info = true;
    }
    else if (elem.level() != candidate_neighbor->level())
    {
      neighbors.insert(candidate_neighbor);
      elem_has_info = candidate_neighbor->level() < elem.level();
    }
    else if (!candidate_neighbor->active())
    {
      // We must be next to an element that has been refined
      mooseAssert(candidate_neighbor->has_children(), "We should have children");

      const auto candidate_neighbor_side = candidate_neighbor->which_neighbor_am_i(&elem);

      for (const auto child_num : make_range(candidate_neighbor->n_children()))
        if (candidate_neighbor->is_child_on_side(child_num, candidate_neighbor_side))
        {
          const Elem * const child = candidate_neighbor->child_ptr(child_num);
          mooseAssert(child->level() - elem.level() == 1, "The math doesn't work out here.");
          mooseAssert(child->has_neighbor(&elem), "Elem should be a neighbor of this child.");
          mooseAssert(child->active(),
                      "We shouldn't have greater than a face mismatch level of one");
          neighbors.insert(child);
        }

      elem_has_info = false;
    }
    else
    {
      neighbors.insert(candidate_neighbor);

      // Both elements are active and they are on the same level, so which one has the info is
      // determined by the lower ID
      elem_has_info = elem.id() < candidate_neighbor->id();
    }

    for (const Elem * const neighbor : neighbors)
    {
      const FaceInfo * const fi =
          elem_has_info ? mesh.faceInfo(&elem, side)
                        : mesh.faceInfo(neighbor, neighbor->which_neighbor_am_i(&elem));

      mooseAssert(fi, "We should have found a FaceInfo");
      mooseAssert(elem_has_info ? &elem == &fi->elem() : &elem == fi->neighborPtr(),
                  "Doesn't seem like we understand how this FaceInfo thing is working");
      mooseAssert(neighbor
                      ? (elem_has_info ? neighbor == fi->neighborPtr() : neighbor == &fi->elem())
                      : true,
                  "Doesn't seem like we understand how this FaceInfo thing is working");

      const Point elem_normal = elem_has_info ? fi->normal() : Point(-fi->normal());

      Real coord;
      MooseMeshUtils::coordTransformFactor(fi->faceCentroid(), coord, coord_type, rz_radial_coord);

      const Point surface_vector = elem_normal * fi->faceArea() * coord;

      act(elem, neighbor, fi, surface_vector, coord, elem_has_info);
    }
  }
}
}
}
