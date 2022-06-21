//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MathFVUtils.h"

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
  /// (gc*elem+(1-gc)*neighbor)+gradient*(rf-rf')
  SkewCorrectedAverage,
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
 * A simple linear interpolation of values between cell centers to a cell face. The \p one_is_elem
 * parameter indicates whether value1 corresponds to the FaceInfo elem value; else it corresponds to
 * the FaceInfo neighbor value
 */
template <typename T, typename T2>
typename libMesh::CompareTypes<T, T2>::supertype
linearInterpolation(const T & value1,
                    const T2 & value2,
                    const FaceInfo & fi,
                    const bool one_is_elem)
{
  mooseAssert(!fi.isBoundary(), "We should not call interpolation on a boundary face!");
  Real coeff = one_is_elem ? fi.gC() : (1. - fi.gC());
  return coeff * (value1 - value2) + value2;
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
  mooseAssert(!fi.isBoundary(), "We should not call interpolation on a boundary face!");
  return linearInterpolation(value1, value2, fi, one_is_elem) +
         face_gradient * fi.skewnessCorrectionVector();
}

/// Provides interpolation of face values for non-advection-specific purposes (although it can/will
/// still be used by advective kernels sometimes). The interpolated value is stored in result.
/// This should be called when a face value needs to be computed from two neighboring
/// cells/elements. value1 and value2 represent the cell property/values from which to compute the
/// face value. The \p one_is_elem parameter indicates whether value1 corresponds to the FaceInfo
/// elem value; else it corresponds to the FaceInfo neighbor value
template <typename T, typename T2, typename T3>
void
interpolate(
    T & result, const T2 & value1, const T3 & value2, const FaceInfo & fi, const bool one_is_elem)
{
  mooseAssert(!fi.isBoundary(), "We should not call interpolation on a boundary face!");
  result = linearInterpolation(value1, value2, fi, one_is_elem);
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

  mooseAssert(!face.fi->isBoundary(), "We should not call interpolation on a boundary face!");

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
  mooseAssert(!fi.isBoundary(), "We should not call interpolation on a boundary face!");
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
      result = (face_advector * fi.normal() > 0) ? fi_elem_advected * fi_elem_advector
                                                 : fi_neighbor_advected * fi_neighbor_advector;
      break;
    }
    default:
      mooseError("Unsupported interpolation method for this interpolation function!");
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
  mooseAssert(!fi.isBoundary(), "We should not call interpolation on a boundary face!");
  switch (m)
  {
    case InterpMethod::Average:
      result = linearInterpolation(value1, value2, fi, one_is_elem);
      break;
    case InterpMethod::Upwind:
    {
      result = ((advector * fi.normal() > 0) == one_is_elem) ? value1 : value2;
      break;
    }
    default:
      mooseError("Unsupported interpolation method for this interpolation function!");
  }
}

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
rF(const Scalar & phiC, const Scalar & phiD, const Vector & gradC, const Point & dCD)
{
  static const auto zero_vec = Point(0);
  if ((phiD - phiC) == 0)
    // Handle zero denominator case. Note that MathUtils::sign returns 1 for sign(0) so we can omit
    // that operation here (e.g. sign(phiD - phiC) = sign(0) = 1). The second term preserves the
    // same sparsity pattern as the else branch; we want to add this so that we don't risk PETSc
    // shrinking the matrix now and then potentially reallocating nonzeros later (which is very
    // slow)
    return 1e6 * MathUtils::sign(gradC * dCD) + zero_vec * gradC;

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
T
interpCoeff(const Limiter<T> & limiter,
            const T & phi_upwind,
            const T & phi_downwind,
            const VectorValue<T> * const grad_phi_upwind,
            const FaceInfo & fi,
            const bool fi_elem_is_upwind)
{
  const auto psi = limiter(
      phi_upwind, phi_downwind, grad_phi_upwind, fi_elem_is_upwind ? fi.dCN() : Point(-fi.dCN()));

  const auto w_f = fi_elem_is_upwind ? fi.gC() : (1. - fi.gC());

  return (1. - psi * (1. - w_f));
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
  mooseAssert(!fi.isBoundary(), "We should not call interpolation on a boundary face!");

  Scalar wf =
      interpCoeff(limiter, phi_upwind, phi_downwind, grad_phi_upwind, fi, fi_elem_is_upwind);

  return wf * (phi_upwind - phi_downwind) + phi_downwind;
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
  mooseAssert(!fi.isBoundary(), "We should not call interpolation on a boundary face!");

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

template <typename T, typename Enable = typename std::enable_if<ScalarTraits<T>::value>::type>
T
interpolate(const FunctorBase<T> & functor, const FaceArg & face)
{
  mooseAssert(face.fi, "this must be non-null");
  mooseAssert(!face.fi->isBoundary(), "We should not call interpolation on a boundary face!");

  auto phi_upwind =
      face.elem_is_upwind ? functor(face.elemFromFace()) : functor(face.neighborFromFace());
  auto phi_downwind =
      face.elem_is_upwind ? functor(face.neighborFromFace()) : functor(face.elemFromFace());

  if (face.limiter_type == LimiterType::Upwind)
    return phi_upwind;
  else if (face.limiter_type == LimiterType::CentralDifference)
    return linearInterpolation(functor, face);
  else
  {
    const auto upwind_arg = face.elem_is_upwind ? face.elemFromFace() : face.neighborFromFace();
    const auto limiter =
        Limiter<typename LimiterValueType<T>::value_type>::build(face.limiter_type);
    const auto grad_phi_upwind = functor.gradient(upwind_arg);

    return interpolate(
        *limiter, phi_upwind, phi_downwind, &grad_phi_upwind, *face.fi, face.elem_is_upwind);
  }
}

template <typename T>
VectorValue<T>
interpolate(const FunctorBase<VectorValue<T>> & functor, const FaceArg & face)
{
  mooseAssert(!face.fi->isBoundary(), "We should not call interpolation on a boundary face!");
  mooseAssert(face.fi, "this must be non-null");

  auto phi_upwind =
      face.elem_is_upwind ? functor(face.elemFromFace()) : functor(face.neighborFromFace());
  auto phi_downwind =
      face.elem_is_upwind ? functor(face.neighborFromFace()) : functor(face.elemFromFace());

  if (face.limiter_type == LimiterType::Upwind)
    return phi_upwind;
  else if (face.limiter_type == LimiterType::CentralDifference)
    return linearInterpolation(functor, face);
  else // We do component-wise limiting for now
  {
    const auto upwind_arg = face.elem_is_upwind ? face.elemFromFace() : face.neighborFromFace();
    const auto limiter =
        Limiter<typename LimiterValueType<T>::value_type>::build(face.limiter_type);
    const auto grad_phi_upwind = functor.gradient(upwind_arg);
    VectorValue<T> ret;
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    {
      const VectorValue<T> & grad = grad_phi_upwind.row(i);
      ret(i) = interpolate(
          *limiter, phi_upwind(i), phi_downwind(i), &grad, *face.fi, face.elem_is_upwind);
    }
    return ret;
  }
}

template <typename T>
T
containerInterpolate(const FunctorBase<T> & functor, const FaceArg & face)
{
  mooseAssert(!face.fi->isBoundary(), "We should not call interpolation on a boundary face!");
  mooseAssert(face.fi, "this must be non-null");

  const auto phi_upwind =
      face.elem_is_upwind ? functor(face.elemFromFace()) : functor(face.neighborFromFace());
  const auto phi_downwind =
      face.elem_is_upwind ? functor(face.neighborFromFace()) : functor(face.elemFromFace());

  // initialize in order to get proper size
  if (face.limiter_type == LimiterType::Upwind)
    return phi_upwind;
  else if (face.limiter_type == LimiterType::CentralDifference)
  {
    T ret = phi_upwind;
    for (const auto i : make_range(ret.size()))
      ret[i] = linearInterpolation(phi_upwind[i], phi_downwind[i], *face.fi, true);
    return ret;
  }
  else
  {
    T ret = phi_upwind;
    const auto limiter = Limiter<typename T::value_type>::build(face.limiter_type);
    const auto upwind_arg = face.elem_is_upwind ? face.elemFromFace() : face.neighborFromFace();
    const auto grad_phi_upwind = functor.gradient(upwind_arg);
    for (const auto i : make_range(ret.size()))
    {
      const auto & grad = grad_phi_upwind[i];
      ret[i] = interpolate(
          *limiter, phi_upwind[i], phi_downwind[i], &grad, *face.fi, face.elem_is_upwind);
      return ret;
    }
  }
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
}
}
