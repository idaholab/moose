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

template <typename>
class MooseVariableFV;

namespace Moose
{
namespace FV
{

/**
 * This function infers based on elements if the faceinfo between them
 * belongs to the element or not.
 * @param elem Reference to an element
 * @param neighbor Pointer to the neighbor of the element
 * @return If the element (first argument) is the owner of the faceinfo between the two elements
 */
bool elemHasFaceInfo(const Elem & elem, const Elem * const neighbor);

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

    bool elem_has_info = elemHasFaceInfo(elem, candidate_neighbor);

    std::set<const Elem *> neighbors;

    const bool inactive_neighbor_detected =
        candidate_neighbor ? !candidate_neighbor->active() : false;

    // See MooseMesh::buildFaceInfo for corresponding checks/additions of FaceInfo
    if (inactive_neighbor_detected)
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
    }
    else
      neighbors.insert(candidate_neighbor);

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

/**
 * This utility determines element one and element two given a \p FaceInfo \p fi and variable \p
 * var. You may ask what in the world "element one" and "element two" means, and that would be a
 * very good question. What it means is: a variable will *always* have degrees of freedom on
 * element one. A variable may or may not have degrees of freedom on element two. So we are
 * introducing a second terminology here. FaceInfo geometric objects have element-neighbor pairs.
 * These element-neighbor pairs are purely geometric and have no relation to the algebraic system
 * of variables. The elem1-elem2 notation introduced here is based on dof/algebraic information
 * and may very well be different from variable to variable, e.g. elem1 may correspond to the
 * FaceInfo elem for one variable (and correspondingly elem2 will be the FaceInfo neighbor), but
 * elem1 may correspond to the FaceInfo neighbor for another variable (and correspondingly for
 * *that* variable elem2 will be the FaceInfo elem).
 * @return A tuple, where the first item is elem1, the second item is elem2, and the third item is
 * a boolean indicating whether elem1 corresponds to the FaceInfo elem
 */
template <typename OutputType>
std::tuple<const Elem *, const Elem *, bool>
determineElemOneAndTwo(const FaceInfo & fi, const MooseVariableFV<OutputType> & var);
}
}
