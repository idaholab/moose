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
    return {elem, &fi, correct_skewness, correct_skewness, elem->subdomain_id()};
  else
  {
    const Elem * const elem_across = (elem == &fi.elem()) ? fi.neighborPtr() : &fi.elem();
    mooseAssert(elem_across && obj.hasBlocks(elem_across->subdomain_id()),
                "How are there no elements with subs on here!");
    return {elem, &fi, correct_skewness, correct_skewness, elem_across->subdomain_id()};
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
 * Make a functor face argument with a central differencing limiter, e.g. compose a face argument
 * that will tell functors to perform (possibly skew-corrected) linear interpolations from cell
 * center values to faces
 * @param fi the face information
 * @param subs the two subdomains that should go into the face argument. The first member of this
 * pair will be the "element" subdomain id and the second member of the pair will be the "neighbor"
 * subdomain id
 * @param skewness_correction whether to apply skew correction weights
 * @param Whether to apply the face gradient when computing a skew corrected face value. A true
 * value for this parameter in conjunction with a false value for \p skewness_correction parameter
 * does not make sense. A false value for this parameter in conjunction with a true value for \p
 * skewness_correction should only be set by someone who really knows what they're doing
 * @return a face argument for functors
 */
inline FaceArg
makeCDFace(const FaceInfo & fi,
           const std::pair<SubdomainID, SubdomainID> & subs,
           const bool skewness_correction = false,
           const bool apply_gradient_correction = false)
{
  return {&fi,
          Moose::FV::LimiterType::CentralDifference,
          true,
          skewness_correction,
          apply_gradient_correction,
          subs.first,
          subs.second};
}

/**
 * Make a functor face argument with a central differencing limiter, e.g. compose a face argument
 * that will tell functors to perform (possibly skew-corrected) linear interpolations from cell
 * center values to faces. The subdomain ids for the face argument will be created from the face
 * information object's \p elem() and \p neighbor() subdomain ids. If the latter is null, then an
 * invalid subdomain ID will be used
 * @param fi the face information
 * @param skewness_correction whether to apply skew correction weights
 * @param Whether to apply the face gradient when computing a skew corrected face value. A true
 * value for this parameter in conjunction with a false value for \p skewness_correction parameter
 * does not make sense. A false value for this parameter in conjunction with a true value for \p
 * skewness_correction should only be set by someone who really knows what they're doing
 * @return a face argument for functors
 */
inline FaceArg
makeCDFace(const FaceInfo & fi,
           const bool skewness_correction = false,
           const bool apply_gradient_correction = false)
{
  return makeCDFace(
      fi,
      std::make_pair(fi.elem().subdomain_id(),
                     fi.neighborPtr() ? fi.neighbor().subdomain_id() : Moose::INVALID_BLOCK_ID),
      skewness_correction,
      apply_gradient_correction);
}

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
  /// (gc*elem+(1-gc)*neighbor)+gradient*(rf-rf')
  SkewCorrectedAverage,
  /// Extended stencil using the vertex values
  VertexBased,
  /// weighted
  Upwind,
  // Rhie-Chow
  RhieChow,
  VanLeer,
  MinMod,
  SOU,
  QUICK
};

/**
 * Produce the interpolation coefficients in the equation:
 *
 * \phi_f = c_1 * \phi_{F1} + c_2 * \phi_{F2}
 *
 * A couple of examples: if we are doing an average interpolation with
 * an orthogonal regular grid, then the pair will be (0.5, 0.5). If we are doing an
 * upwind interpolation with the velocity facing outward from the F1 element,
 * then the pair will be (1.0, 0.0).
 *
 * @param m The interpolation method
 * @param fi The face information
 * @param one_is_elem Whether fi.elem() == F1
 * @param advector The advecting velocity. Not relevant for an Average interpolation
 * @return a pair where the first Real is c_1 and the second Real is c_2
 */
template <typename Vector = RealVectorValue>
std::pair<Real, Real>
interpCoeffs(const InterpMethod m,
             const FaceInfo & fi,
             const bool one_is_elem,
             const Vector advector = Vector())
{
  switch (m)
  {
    case InterpMethod::Average:
    {
      if (one_is_elem)
        return std::make_pair(fi.gC(), 1. - fi.gC());
      else
        return std::make_pair(1. - fi.gC(), fi.gC());
    }

    case InterpMethod::SkewCorrectedAverage:
    {
      if (one_is_elem)
        return std::make_pair(fi.gCSkewed(), 1. - fi.gCSkewed());
      else
        return std::make_pair(1. - fi.gCSkewed(), fi.gCSkewed());
    }

    case InterpMethod::Upwind:
    {
      if ((advector * fi.normal() > 0) == one_is_elem)
        return std::make_pair(1., 0.);
      else
        return std::make_pair(0., 1.);
    }

    default:
      mooseError("Unrecognized interpolation method");
  }
}

/**
 * A simple linear interpolation of values between cell centers to a cell face. The \p one_is_elem
 * parameter indicates whether value1 corresponds to the FaceInfo elem value; else it corresponds to
 * the FaceInfo neighbor value
 */
template <typename T, typename T2>
typename libMesh::CompareTypes<T, T2>::supertype
linearInterpolation(const T & value1,
                    const T2 & value2,
                    const FaceInfo & fi,
                    const bool one_is_elem,
                    const InterpMethod interp_method = InterpMethod::Average)
{
  mooseAssert(interp_method == InterpMethod::Average ||
                  interp_method == InterpMethod::SkewCorrectedAverage,
              "The selected interpolation function only works with average or skewness-corrected "
              "average options!");
  const auto coeffs = interpCoeffs(interp_method, fi, one_is_elem);
  return coeffs.first * value1 + coeffs.second * value2;
}

/**
 * Linear interpolation with skewness correction using the face gradient.
 * See more info in Moukalled Chapter 9. The correction involves a first order
 * Taylor expansion around the intersection of the cell face and the line
 * connecting the two cell centers.
 */
template <typename T, typename T2, typename T3>
typename libMesh::CompareTypes<T, T2>::supertype
skewCorrectedLinearInterpolation(const T & value1,
                                 const T2 & value2,
                                 const T3 & face_gradient,
                                 const FaceInfo & fi,
                                 const bool one_is_elem)
{
  const auto coeffs = interpCoeffs(InterpMethod::SkewCorrectedAverage, fi, one_is_elem);

  auto value = (coeffs.first * value1 + coeffs.second * value2) +
               face_gradient * (fi.faceCentroid() - fi.rIntersection());
  return value;
}

/// Provides interpolation of face values for non-advection-specific purposes (although it can/will
/// still be used by advective kernels sometimes). The interpolated value is stored in result.
/// This should be called when a face value needs to be computed from two neighboring
/// cells/elements. value1 and value2 represent the cell property/values from which to compute the
/// face value. The \p one_is_elem parameter indicates whether value1 corresponds to the FaceInfo
/// elem value; else it corresponds to the FaceInfo neighbor value
template <typename T, typename T2, typename T3>
void
interpolate(InterpMethod m,
            T & result,
            const T2 & value1,
            const T3 & value2,
            const FaceInfo & fi,
            const bool one_is_elem)
{
  switch (m)
  {
    case InterpMethod::Average:
      result = linearInterpolation(value1, value2, fi, one_is_elem);
      break;
    case InterpMethod::SkewCorrectedAverage:
    {
      // We create a zero gradient to ensure that the skewness-corrected
      // weights are used, but no correction is applied. This will change when the
      // old weights are replaced by the ones used with skewness-correction
      typename TensorTools::IncrementRank<T2>::type surface_gradient;
      result = skewCorrectedLinearInterpolation(value1, value2, surface_gradient, fi, one_is_elem);
      break;
    }
    default:
      mooseError("unsupported interpolation method for FVFaceInterface::interpolate");
  }
}

/**
 * perform a possibly skew-corrected linear interpolation by evaluating the supplied functor with
 * the provided functor face argument
 */
template <typename T>
T
linearInterpolation(const FunctorBase<T> & functor, const FaceArg & face)
{
  mooseAssert(face.limiter_type == LimiterType::CentralDifference,
              "this interpolation method is meant for linear interpolations");

  mooseAssert(face.fi,
              "We must have a non-null face_info in order to prepare our ElemFromFace tuples");

  const auto elem_from_face = face.elemFromFace();
  const auto neighbor_from_face = face.neighborFromFace();

  if (face.correct_skewness)
  {
    typedef typename TensorTools::IncrementRank<T>::type GradientType;
    // This condition ensures that the recursive algorithm (face_center->
    // face_gradient -> cell_gradient -> face_center -> ...) terminates after
    // one loop. It is hardcoded to one loop at this point since it yields
    // 2nd order accuracy on skewed meshes with the minimum additional effort.
    FaceArg new_face(face);
    new_face.apply_gradient_to_skewness = false;
    const auto surface_gradient =
        face.apply_gradient_to_skewness ? functor.gradient(new_face) : GradientType(0);

    return skewCorrectedLinearInterpolation(
        functor(elem_from_face), functor(neighbor_from_face), surface_gradient, *face.fi, true);
  }
  else
    return linearInterpolation(
        functor(elem_from_face), functor(neighbor_from_face), *face.fi, true);
}

/**
 * Computes the product of the advected and the advector based on the given interpolation method
 */
template <typename T1,
          typename T2,
          typename T3,
          template <typename>
          class Vector1,
          template <typename>
          class Vector2>
void
interpolate(InterpMethod m,
            Vector1<T1> & result,
            const T2 & fi_elem_advected,
            const T2 & fi_neighbor_advected,
            const Vector2<T3> & fi_elem_advector,
            const Vector2<T3> & fi_neighbor_advector,
            const FaceInfo & fi)
{
  switch (m)
  {
    case InterpMethod::Average:
      result = linearInterpolation(fi_elem_advected * fi_elem_advector,
                                   fi_neighbor_advected * fi_neighbor_advector,
                                   fi,
                                   true);
      break;
    case InterpMethod::Upwind:
    {
      const auto face_advector = linearInterpolation(MetaPhysicL::raw_value(fi_elem_advector),
                                                     MetaPhysicL::raw_value(fi_neighbor_advector),
                                                     fi,
                                                     true);
      Real elem_coeff, neighbor_coeff;
      if (face_advector * fi.normal() > 0)
        elem_coeff = 1, neighbor_coeff = 0;
      else
        elem_coeff = 0, neighbor_coeff = 1;

      result = elem_coeff * fi_elem_advected * fi_elem_advector +
               neighbor_coeff * fi_neighbor_advected * fi_neighbor_advector;
      break;
    }
    default:
      mooseError("unsupported interpolation method for FVFaceInterface::interpolate");
  }
}

/// Provides interpolation of face values for advective flux kernels.  This should be called by
/// advective kernels when a face value is needed from two neighboring cells/elements.  The
/// interpolated value is stored in result. value1 and value2 represent the two neighboring advected
/// cell property/values. advector represents the vector quantity at the face that is doing the
/// advecting (e.g. the flow velocity at the face); this value often will have been computed using a
/// call to the non-advective interpolate function. The \p one_is_elem parameter indicates whether
/// value1 corresponds to the FaceInfo elem value; else it corresponds to the FaceInfo neighbor
/// value
template <typename T, typename T2, typename T3, typename Vector>
void
interpolate(InterpMethod m,
            T & result,
            const T2 & value1,
            const T3 & value2,
            const Vector & advector,
            const FaceInfo & fi,
            const bool one_is_elem)
{
  const auto coeffs = interpCoeffs(m, fi, one_is_elem, advector);
  result = coeffs.first * value1 + coeffs.second * value2;
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
 * From Moukalled 12.30
 *
 * r_f = (phiC - phiU) / (phiD - phiC)
 *
 * However, this formula is only clear on grids where upwind locations can be readily determined,
 * which is not the case for unstructured meshes. So we leverage a virtual upwind location and
 * Moukalled 12.65
 *
 * phiD - phiU = 2 * grad(phi)_C * dCD ->
 * phiU = phiD - 2 * grad(phi)_C * dCD
 *
 * Combining the two equations and performing some algebraic manipulation yields this equation for
 * r_f:
 *
 * r_f = 2 * grad(phi)_C * dCD / (phiD - phiC) - 1
 *
 * This equation is clearly asymmetric considering the face between C and D because of the
 * subscript on grad(phi). Hence this method can be thought of as constructing an r associated with
 * the C side of the face
 */
template <typename Scalar, typename Vector>
Scalar
rF(const Scalar & phiC, const Scalar & phiD, const Vector & gradC, const RealVectorValue & dCD)
{
  if ((phiD - phiC) == 0)
    return 1e6 * MathUtils::sign(gradC * dCD) + 0 * (gradC * dCD + phiD + phiC);

  return 2. * gradC * dCD / (phiD - phiC) - 1.;
}

/**
 * Produce the interpolation coefficients in the equation:
 *
 * \phi_f = c_upwind * \phi_{upwind} + c_downwind * \phi_{downwind}
 *
 * A couple of examples: if we are doing an average interpolation with
 * an orthogonal regular grid, then the pair will be (0.5, 0.5). If we are doing an
 * upwind interpolation then the pair will be (1.0, 0.0).
 *
 * @return a pair where the first Real is c_upwind and the second Real is c_downwind
 */
template <typename T>
std::pair<T, T>
interpCoeffs(const Limiter<T> & limiter,
             const T & phi_upwind,
             const T & phi_downwind,
             const VectorValue<T> * const grad_phi_upwind,
             const FaceInfo & fi,
             const bool fi_elem_is_upwind)
{
  // Using beta, w_f, g nomenclature from Greenshields
  const auto beta = limiter(phi_upwind,
                            phi_downwind,
                            grad_phi_upwind,
                            fi_elem_is_upwind ? fi.dCF() : RealVectorValue(-fi.dCF()));

  const auto w_f = fi_elem_is_upwind ? fi.gC() : (1. - fi.gC());

  const auto g = beta * (1. - w_f);

  return std::make_pair(1. - g, g);
}

template <typename T>
struct GradientType
{
};

/**
 * Interpolates with a limiter
 */
template <typename Scalar,
          typename Vector,
          typename Enable = typename std::enable_if<ScalarTraits<Scalar>::value>::type>
Scalar
interpolate(const Limiter<Scalar> & limiter,
            const Scalar & phi_upwind,
            const Scalar & phi_downwind,
            const Vector * const grad_phi_upwind,
            const FaceInfo & fi,
            const bool fi_elem_is_upwind)
{
  auto pr = interpCoeffs(limiter, phi_upwind, phi_downwind, grad_phi_upwind, fi, fi_elem_is_upwind);
  return pr.first * phi_upwind + pr.second * phi_downwind;
}

/**
 * Vector overload
 */
template <typename Limiter, typename T, typename Tensor>
VectorValue<T>
interpolate(const Limiter & limiter,
            const TypeVector<T> & phi_upwind,
            const TypeVector<T> & phi_downwind,
            const Tensor * const /*grad_phi_upwind*/,
            const FaceInfo & fi,
            const bool fi_elem_is_upwind)
{
  mooseAssert(limiter.constant(),
              "Non-constant limiters are not currently supported in the vector overload of the "
              "limited interpolate method");
  static const VectorValue<T> example_gradient(0);

  VectorValue<T> ret;
  for (const auto i : make_range(unsigned(LIBMESH_DIM)))
    ret(i) = interpolate(
        limiter, phi_upwind(i), phi_downwind(i), &example_gradient, fi, fi_elem_is_upwind);

  return ret;
}

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
