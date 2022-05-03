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
    case InterpMethod::SkewCorrectedAverage:
    {
      if (one_is_elem)
        return std::make_pair(fi.gC(), 1. - fi.gC());
      else
        return std::make_pair(1. - fi.gC(), fi.gC());
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
    case InterpMethod::SkewCorrectedAverage:
      result = linearInterpolation(value1, value2, fi, one_is_elem);
      break;
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
    // This condition ensures that the recursive algorithm (face_center->
    // face_gradient -> cell_gradient -> face_center -> ...) terminates after
    // one loop. It is hardcoded to one loop at this point since it yields
    // 2nd order accuracy on skewed meshes with the minimum additional effort.
    FaceArg new_face(face);
    new_face.correct_skewness = false;
    const auto surface_gradient = functor.gradient(new_face);

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
  const auto fgrad = phiD - phiC;
  const auto fgradC = gradC * dCD;
  constexpr Real coeff = 1e3;
  static const auto zero_vec = RealVectorValue(0);

  if (std::abs(fgradC) >= coeff * std::abs(fgrad))
    // Second term (multiplied by zero) is to keep same sparsity pattern as else branch
    return 2 * coeff * MathUtils::sign(fgradC) * MathUtils::sign(fgrad) + zero_vec * gradC;

  return 2. * fgradC / fgrad - 1.;
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
            const Tensor * const grad_phi_upwind,
            const FaceInfo & fi,
            const bool fi_elem_is_upwind)
{
  mooseAssert(limiter.constant() || grad_phi_upwind,
              "Non-null gradient only supported for constant limiters.");

  const VectorValue<T> * const gradient_example = nullptr;
  VectorValue<T> ret;
  for (const auto i : make_range(unsigned(LIBMESH_DIM)))
  {
    if (grad_phi_upwind)
    {
      const VectorValue<T> gradient = grad_phi_upwind->row(i);
      ret(i) =
          interpolate(limiter, phi_upwind(i), phi_downwind(i), &gradient, fi, fi_elem_is_upwind);
    }
    else
      ret(i) = interpolate(
          limiter, phi_upwind(i), phi_downwind(i), gradient_example, fi, fi_elem_is_upwind);
  }

  return ret;
}

/**
 * Interpolates with a limiter and a face argument
 * @return a pair of pairs. The first pair corresponds to the interpolation coefficients with the
 * first corresponding to the face information element and the second corresponding to the face
 * information neighbor. This pair should sum to unity. The second pair corresponds to the face
 * information functor element value and neighbor
 */
template <typename T, typename Enable = typename std::enable_if<ScalarTraits<T>::value>::type>
std::pair<std::pair<T, T>, std::pair<T, T>>
interpCoeffsAndAdvected(const FunctorBase<T> & functor, const FaceArg & face)
{
  typedef typename FunctorBase<T>::GradientType GradientType;
  static const GradientType zero(0);

  mooseAssert(face.fi, "this must be non-null");
  const auto limiter = Limiter<typename LimiterValueType<T>::value_type>::build(face.limiter_type);

  const auto upwind_arg = face.elem_is_upwind ? face.elemFromFace() : face.neighborFromFace();
  const auto downwind_arg = face.elem_is_upwind ? face.neighborFromFace() : face.elemFromFace();
  auto phi_upwind = functor(upwind_arg);
  auto phi_downwind = functor(downwind_arg);

  std::pair<T, T> interp_coeffs;
  if (face.limiter_type == LimiterType::Upwind ||
      face.limiter_type == LimiterType::CentralDifference)
    interp_coeffs =
        interpCoeffs(*limiter, phi_upwind, phi_downwind, &zero, *face.fi, face.elem_is_upwind);
  else
  {
    const auto grad_phi_upwind = functor.gradient(upwind_arg);
    interp_coeffs = interpCoeffs(
        *limiter, phi_upwind, phi_downwind, &grad_phi_upwind, *face.fi, face.elem_is_upwind);
  }

  if (face.elem_is_upwind)
    return std::make_pair(std::move(interp_coeffs),
                          std::make_pair(std::move(phi_upwind), std::move(phi_downwind)));
  else
    return std::make_pair(
        std::make_pair(std::move(interp_coeffs.second), std::move(interp_coeffs.first)),
        std::make_pair(std::move(phi_downwind), std::move(phi_upwind)));
}

template <typename T, typename Enable = typename std::enable_if<ScalarTraits<T>::value>::type>
T
interpolate(const FunctorBase<T> & functor, const FaceArg & face)
{
  // Special handling for central differencing as it is the only interpolation method which
  // currently supports skew correction
  if (face.limiter_type == LimiterType::CentralDifference)
    return linearInterpolation(functor, face);

  const auto [interp_coeffs, advected] = interpCoeffsAndAdvected(functor, face);
  return interp_coeffs.first * advected.first + interp_coeffs.second * advected.second;
}

template <typename T>
VectorValue<T>
interpolate(const FunctorBase<VectorValue<T>> & functor, const FaceArg & face)
{
  static const VectorValue<T> grad_zero(0);

  mooseAssert(face.fi, "this must be non-null");
  const auto limiter = Limiter<typename LimiterValueType<T>::value_type>::build(face.limiter_type);

  const auto upwind_arg = face.elem_is_upwind ? face.elemFromFace() : face.neighborFromFace();
  const auto downwind_arg = face.elem_is_upwind ? face.neighborFromFace() : face.elemFromFace();
  auto phi_upwind = functor(upwind_arg);
  auto phi_downwind = functor(downwind_arg);

  VectorValue<T> ret;
  T coeff_upwind, coeff_downwind;

  if (face.limiter_type == LimiterType::Upwind ||
      face.limiter_type == LimiterType::CentralDifference)
  {
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    {
      const auto &component_upwind = phi_upwind(i), component_downwind = phi_downwind(i);
      std::tie(coeff_upwind, coeff_downwind) = interpCoeffs(*limiter,
                                                            component_upwind,
                                                            component_downwind,
                                                            &grad_zero,
                                                            *face.fi,
                                                            face.elem_is_upwind);
      ret(i) = coeff_upwind * component_upwind + coeff_downwind * component_downwind;
    }
  }
  else
  {
    const auto grad_phi_upwind = functor.gradient(upwind_arg);
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    {
      const auto &component_upwind = phi_upwind(i), component_downwind = phi_downwind(i);
      const VectorValue<T> grad = grad_phi_upwind.row(i);
      std::tie(coeff_upwind, coeff_downwind) = interpCoeffs(
          *limiter, component_upwind, component_downwind, &grad, *face.fi, face.elem_is_upwind);
      ret(i) = coeff_upwind * component_upwind + coeff_downwind * component_downwind;
    }
  }

  return ret;
}

template <typename T>
T
containerInterpolate(const FunctorBase<T> & functor, const FaceArg & face)
{
  typedef typename FunctorBase<T>::GradientType ContainerGradientType;
  typedef typename ContainerGradientType::value_type GradientType;
  const GradientType * const example_gradient = nullptr;

  mooseAssert(face.fi, "this must be non-null");
  const auto limiter = Limiter<typename T::value_type>::build(face.limiter_type);

  const auto upwind_arg = face.elem_is_upwind ? face.elemFromFace() : face.neighborFromFace();
  const auto downwind_arg = face.elem_is_upwind ? face.neighborFromFace() : face.elemFromFace();
  const auto phi_upwind = functor(upwind_arg);
  const auto phi_downwind = functor(downwind_arg);

  // initialize in order to get proper size
  T ret = phi_upwind;
  typename T::value_type coeff_upwind, coeff_downwind;

  if (face.limiter_type == LimiterType::Upwind ||
      face.limiter_type == LimiterType::CentralDifference)
  {
    for (const auto i : make_range(ret.size()))
    {
      const auto &component_upwind = phi_upwind[i], component_downwind = phi_downwind[i];
      std::tie(coeff_upwind, coeff_downwind) = interpCoeffs(*limiter,
                                                            component_upwind,
                                                            component_downwind,
                                                            example_gradient,
                                                            *face.fi,
                                                            face.elem_is_upwind);
      ret[i] = coeff_upwind * component_upwind + coeff_downwind * component_downwind;
    }
  }
  else
  {
    const auto grad_phi_upwind = functor.gradient(upwind_arg);
    for (const auto i : make_range(ret.size()))
    {
      const auto &component_upwind = phi_upwind[i], component_downwind = phi_downwind[i];
      const auto & grad = grad_phi_upwind[i];
      std::tie(coeff_upwind, coeff_downwind) = interpCoeffs(
          *limiter, component_upwind, component_downwind, &grad, *face.fi, face.elem_is_upwind);
      ret[i] = coeff_upwind * component_upwind + coeff_downwind * component_downwind;
    }
  }

  return ret;
}

template <typename T>
std::vector<T>
interpolate(const FunctorBase<std::vector<T>> & functor, const FaceArg & face)
{
  return containerInterpolate(functor, face);
}

template <typename T, std::size_t N>
std::array<T, N>
interpolate(const FunctorBase<std::array<T, N>> & functor, const FaceArg & face)
{
  return containerInterpolate(functor, face);
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
