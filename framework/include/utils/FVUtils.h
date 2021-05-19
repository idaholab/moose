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
#include "Limiter.h"
#include "MathUtils.h"
#include "libmesh/elem.h"
#include "libmesh/compare_types.h"

#include <tuple>

template <typename>
class MooseVariableFV;

namespace Moose
{
namespace FV
{
class Limiter;

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
                    const bool one_is_elem)
{
  const auto coeffs = interpCoeffs(InterpMethod::Average, fi, one_is_elem);
  return coeffs.first * value1 + coeffs.second * value2;
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
    default:
      mooseError("unsupported interpolation method for FVFaceInterface::interpolate");
  }
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

/**
 * This utility determines element one and element two given a \p FaceInfo \p fi and variable \p
 * var. You may ask what in the world "element one" and "element two" means, and that would be a
 * very good question. What it means is: a variable will *always* have degrees of freedom on element
 * one. A variable may or may not have degrees of freedom on element two. So we are introducing a
 * second terminology here. FaceInfo geometric objects have element-neighbor pairs. These
 * element-neighbor pairs are purely geometric and have no relation to the algebraic system of
 * variables. The elem1-elem2 notation introduced here is based on dof/algebraic information and may
 * very well be different from variable to variable, e.g. elem1 may correspond to the FaceInfo elem
 * for one variable (and correspondingly elem2 will be the FaceInfo neighbor), but elem1 may
 * correspond to the FaceInfo neighbor for another variable (and correspondingly for *that* variable
 * elem2 will be the FaceInfo elem).
 * @return A tuple, where the first item is elem1, the second item is elem2, and the third item is a
 * boolean indicating whether elem1 corresponds to the FaceInfo elem
 */
template <typename OutputType>
std::tuple<const Elem *, const Elem *, bool>
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

  return std::make_tuple(elem_one, elem_two, one_is_elem);
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
ADReal
rF(const Scalar & phiC, const Scalar & phiD, const Vector & gradC, const RealVectorValue & dCD)
{
  if ((phiD - phiC) == 0)
    return 1e6 * MathUtils::sign(gradC * dCD) + 0 * (gradC * dCD + phiD + phiC);

  return 2. * gradC * dCD / (phiD - phiC) - 1.;
}

/**
 * Interpolates with a limiter
 */
template <typename Scalar, typename Vector>
ADReal
interpolate(const Limiter & limiter,
            const Scalar & phiC,
            const Scalar & phiD,
            const Vector & gradC,
            const FaceInfo & fi,
            const bool C_is_elem)
{
  // Using beta, w_f, g nomenclature from Greenshields
  const auto r_f = rF(phiC, phiD, gradC, C_is_elem ? fi.dCF() : RealVectorValue(-fi.dCF()));
  const auto beta = limiter(r_f);

  const auto w_f = C_is_elem ? fi.gC() : (1. - fi.gC());

  const auto g = beta * (1. - w_f);

  return (1. - g) * phiC + g * phiD;
}

}
}
