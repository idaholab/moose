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
#include "SubProblem.h"
#include "Assembly.h"
#include "FaceInfo.h"
#include "MooseVariableFV.h"
#include "libmesh/elem.h"
#include "libmesh/compare_types.h"

#include <utility>

template <typename>
class MooseVariableFV;

namespace Moose
{
namespace FV
{
/// This codifies a set of available ways to interpolate with elem+neighbor
/// solution information to calculate values (e.g. solution, material
/// properties, etc.) at the face (centroid).  These methods are used in the
/// class's interpolate functions.  Some interpolation methods are only meant
/// to be used with advective terms (e.g. upwind), others are more
/// generically applicable.
enum class InterpMethod
{
  /// (elem+neighbor)/2
  Average,
  /// weighted
  Upwind,
  // Rhie-Chow
  RhieChow
};

/**
 * A simple linear interpolation of values between cell centers to a cell face
 */
template <typename T, typename T2>
typename libMesh::CompareTypes<T, T2>::supertype
linearInterpolation(const T & elem_value, const T2 & neighbor_value, const FaceInfo & fi)
{
  return fi.gC() * elem_value + (1. - fi.gC()) * neighbor_value;
}

/**
 * A simple linear interpolation of values between cell centers to a cell face
 */
template <typename T, typename T2>
libMesh::VectorValue<typename libMesh::CompareTypes<T, T2>::supertype>
linearInterpolation(const libMesh::VectorValue<T> & elem_value,
                    const libMesh::VectorValue<T2> & neighbor_value,
                    const FaceInfo & fi)
{
  return fi.gC() * elem_value + (1. - fi.gC()) * neighbor_value;
}

/// Provides interpolation of face values for non-advection-specific purposes
/// (although it can/will still be used by advective kernels sometimes).  The
/// interpolated value is stored in result.  This should be called when a
/// face value needs to be computed using elem and neighbor information (e.g. a
/// material property, solution value, etc.).  elem and neighbor represent the
/// property/value to compute the face value for.
template <typename T, typename T2, typename T3>
void
interpolate(InterpMethod m, T & result, const T2 & elem, const T3 & neighbor, const FaceInfo & fi)
{
  switch (m)
  {
    case InterpMethod::Average:
      result = linearInterpolation(elem, neighbor, fi);
      break;
    default:
      mooseError("unsupported interpolation method for FVFaceInterface::interpolate");
  }
}

/// Provides interpolation of face values for advective flux kernels.  This
/// should be called by advective kernels when a u_face value is needed from
/// u_elem and u_neighbor.  The interpolated value is stored in result.  elem
/// and neighbor represent the property/value being advected in the elem and
/// neighbor elements respectively.  advector represents the vector quantity at
/// the face that is doing the advecting (e.g. the flow velocity at the
/// face); this value often will have been computed using a call to the
/// non-advective interpolate function.
template <typename T, typename T2, typename T3, typename Vector>
void
interpolate(InterpMethod m,
            T & result,
            const T2 & elem,
            const T3 & neighbor,
            const Vector & advector,
            const FaceInfo & fi)
{
  switch (m)
  {
    case InterpMethod::Average:
      result = linearInterpolation(elem, neighbor, fi);
      break;
    case InterpMethod::Upwind:
      if (advector * fi.normal() > 0)
        result = elem;
      else
        result = neighbor;
      break;
    default:
      mooseError("unsupported interpolation method for FVFaceInterface::interpolate");
  }
}

template <typename ActionFunctor>
void
loopOverElemFaceInfo(const Elem & elem,
                     const MooseMesh & mesh,
                     const SubProblem & subproblem,
                     ActionFunctor & act)
{
  mooseAssert(elem.active(), "We should never call this method with an inactive element");

  for (const auto side : elem.side_index_range())
  {
    const Elem * const candidate_neighbor = elem.neighbor_ptr(side);

    bool elem_has_info;

    std::set<const Elem *> neighbors;

    // See MooseMesh::buildFaceInfo for corresponding checks/additions of FaceInfo
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

      mooseAssert(neighbor ? subproblem.getCoordSystem(elem.subdomain_id()) ==
                                 subproblem.getCoordSystem(neighbor->subdomain_id())
                           : true,
                  "Coordinate systems must be the same between element and neighbor");

      Real coord;
      coordTransformFactor(subproblem, elem.subdomain_id(), fi->faceCentroid(), coord);

      const Point surface_vector = elem_normal * fi->faceArea() * coord;

      act(elem, neighbor, fi, surface_vector, coord, elem_has_info);
    }
  }
}

/// Calculates and returns "grad_u dot normal" on the face to be used for
/// diffusive terms.  If using any cross-diffusion corrections, etc. all
/// those calculations should be handled appropriately by this function.
template <typename T, typename T2>
ADReal gradUDotNormal(const T & elem_value,
                      const T2 & neighbor_value,
                      const FaceInfo & face_info,
                      const MooseVariableFV<Real> & fv_var);

template <typename OutputType>
std::pair<const Elem *, const Elem *>
determineElemOneAndTwo(const FaceInfo & fi, const MooseVariableFV<OutputType> & var)
{
  auto ft = fi.faceType(var.name());
  mooseAssert(
      ft == FaceInfo::VarFaceNeighbors::BOTH
          ? var.hasBlocks(fi.elem().subdomain_id()) && fi.neighborPtr() &&
                var.hasBlocks(fi.neighborPtr()->subdomain_id())
          : true,
      "Finite volume variable "
          << var.name()
          << " does not exist on both sides of the face despite what the FaceInfo is telling us.");
  mooseAssert(ft == FaceInfo::VarFaceNeighbors::ELEM
                  ? var.hasBlocks(fi.elem().subdomain_id()) &&
                        (!fi.neighborPtr() || !var.hasBlocks(fi.neighborPtr()->subdomain_id()))
                  : true,
              "Finite volume variable " << var.name()
                                        << " does not exist on or only on the elem side of the "
                                           "face despite what the FaceInfo is telling us.");
  mooseAssert(ft == FaceInfo::VarFaceNeighbors::NEIGHBOR
                  ? fi.neighborPtr() && var.hasBlocks(fi.neighborPtr()->subdomain_id()) &&
                        !var.hasBlocks(fi.elem().subdomain_id())
                  : true,
              "Finite volume variable " << var.name()
                                        << " does not exist on or only on the neighbor side of the "
                                           "face despite what the FaceInfo is telling us.");

  bool one_is_elem =
      ft == FaceInfo::VarFaceNeighbors::BOTH || ft == FaceInfo::VarFaceNeighbors::ELEM;
  const Elem * const elem_one = one_is_elem ? &fi.elem() : fi.neighborPtr();
  mooseAssert(elem_one, "This elem should be non-null!");
  const Elem * const elem_two = one_is_elem ? fi.neighborPtr() : &fi.elem();

  return std::make_pair(elem_one, elem_two);
}
}

}
