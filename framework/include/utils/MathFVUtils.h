//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"
#include "FaceInfo.h"
#include "Limiter.h"
#include "MathUtils.h"
#include "MooseFunctor.h"
#include "libmesh/compare_types.h"
#include "libmesh/elem.h"
#include <tuple>

template <typename>
class MooseVariableFV;

namespace Moose
{
namespace FV
{
/**
 * This creates a structure with info on : an element, \p FaceInfo, skewness correction and
 * subdomain ID. The element returned will correspond to the method argument. The \p FaceInfo part
 * of the structure will simply correspond to the current \p _face_info. The subdomain ID part of
 * the structure will correspond to the subdomain ID of the method's element argument except in the
 * case in which that subdomain ID does not correspond to a subdomain ID that the \p obj is defined
 * on. In that case the subdomain ID of the structure will correspond to the subdomain ID of the
 * element across the face, on which the \p obj *is* defined
 */
template <typename SubdomainRestrictable>
ElemFromFaceArg
makeSidedFace(const SubdomainRestrictable & obj,
              const Elem * const elem,
              const FaceInfo & fi,
              const bool correct_skewness = false)
{
  if (elem && obj.hasBlocks(elem->subdomain_id()))
    return {elem, &fi, correct_skewness, elem->subdomain_id()};
  else
  {
    const Elem * const elem_across = (elem == &fi.elem()) ? fi.neighborPtr() : &fi.elem();
    mooseAssert(elem_across && obj.hasBlocks(elem_across->subdomain_id()),
                "How are there no elements with subs on here!");
    return {elem, &fi, correct_skewness, elem_across->subdomain_id()};
  }
}

/**
 * @return the value of \p makeSidedFace called with the face info element
 */
template <typename SubdomainRestrictable>
ElemFromFaceArg
elemFromFace(const SubdomainRestrictable & obj,
             const FaceInfo & fi,
             const bool correct_skewness = false)
{
  return makeSidedFace(obj, &fi.elem(), fi, correct_skewness);
}

/**
 * @return the value of \p makeSidedFace called with the face info neighbor
 */
template <typename SubdomainRestrictable>
ElemFromFaceArg
neighborFromFace(const SubdomainRestrictable & obj,
                 const FaceInfo & fi,
                 const bool correct_skewness = false)
{
  return makeSidedFace(obj, fi.neighborPtr(), fi, correct_skewness);
}

/**
 * Determine the subdomain ID pair that should be used when creating a face argument for a
 * functor. As explained in the doxygen for \p makeSidedFace these
 * subdomain IDs do not simply correspond to the subdomain IDs of the face information element pair;
 * they must respect the block restriction of the \p obj
 */
template <typename SubdomainRestrictable>
std::pair<SubdomainID, SubdomainID>
faceArgSubdomains(const SubdomainRestrictable & obj, const FaceInfo & fi)
{
  return std::make_pair(makeSidedFace(obj, &fi.elem(), fi).sub_id,
                        makeSidedFace(obj, fi.neighborPtr(), fi).sub_id);
}

/**
 * Create a functor face argument from provided component arguments
 * @param fi the face information object
 * @param limiter_type the limiter that defines how to perform interpolations to the faces
 * @param elem_is_upwind whether the face information element is the upwind element (the value of
 * this doesn't matter when the limiter type is CentralDifference)
 * @param subs the two subdomain ids that should go into the face argument. These may not always
 * correspond to the face information element and neighbor subdomain ids (for instance if we are on
 * a boundary)
 * @param correct_skewness whether to apply skew correction
 * @return the functor face argument
 */
inline FaceArg
makeFace(const FaceInfo & fi,
         const LimiterType limiter_type,
         const bool elem_is_upwind,
         const std::pair<SubdomainID, SubdomainID> & subs,
         const bool correct_skewness = false)
{
  return {&fi, limiter_type, elem_is_upwind, correct_skewness, subs.first, subs.second};
}

/**
 * Make a functor face argument with a central differencing limiter, e.g. compose a face argument
 * that will tell functors to perform (possibly skew-corrected) linear interpolations from cell
 * center values to faces
 * @param fi the face information
 * @param subs the two subdomains that should go into the face argument. The first member of this
 * pair will be the "element" subdomain id and the second member of the pair will be the "neighbor"
 * subdomain id
 * @param correct_skewness whether to apply skew correction
 * @return a face argument for functors
 */
inline FaceArg
makeCDFace(const FaceInfo & fi,
           const std::pair<SubdomainID, SubdomainID> & subs,
           const bool correct_skewness = false)
{
  return makeFace(fi, LimiterType::CentralDifference, true, subs, correct_skewness);
}

/**
 * Make a functor face argument with a central differencing limiter, e.g. compose a face argument
 * that will tell functors to perform (possibly skew-corrected) linear interpolations from cell
 * center values to faces. The subdomain ids for the face argument will be created from the face
 * information object's \p elem() and \p neighbor() subdomain ids. If the latter is null, then an
 * invalid subdomain ID will be used
 * @param fi the face information
 * @param apply_gradient_so_skewness whether to apply skew correction
 * @return a face argument for functors
 */
inline FaceArg
makeCDFace(const FaceInfo & fi, const bool correct_skewness = false)
{
  return makeCDFace(
      fi,
      std::make_pair(fi.elem().subdomain_id(),
                     fi.neighborPtr() ? fi.neighbor().subdomain_id() : Moose::INVALID_BLOCK_ID),
      correct_skewness);
}

/// Calculates and returns "grad_u dot normal" on the face to be used for
/// diffusive terms.  If using any cross-diffusion corrections, etc. all
/// those calculations should be handled appropriately by this function.
template <typename T, typename T2>
ADReal gradUDotNormal(const T & elem_value,
                      const T2 & neighbor_value,
                      const FaceInfo & face_info,
                      const MooseVariableFV<Real> & fv_var,
                      bool correct_skewness = false);

/**
 * Return whether the supplied face is on a boundary of the \p object's execution
 */
template <typename SubdomainRestrictable>
bool
onBoundary(const SubdomainRestrictable & obj, const FaceInfo & fi)
{
  const bool internal = fi.neighborPtr() && obj.hasBlocks(fi.elemSubdomainID()) &&
                        obj.hasBlocks(fi.neighborSubdomainID());
  return !internal;
}

/**
 * Determine whether the passed-in face is on the boundary of an object that lives on the provided
 * subdomains. Note that if the subdomain set is empty we consider that to mean that the object has
 * no block restriction and lives on the entire mesh
 */
bool onBoundary(const std::set<SubdomainID> & subs, const FaceInfo & fi);
}
}
