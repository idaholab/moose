//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarUtils.h"
#include "MooseLagrangeHelpers.h"

#include "libmesh/enum_to_string.h"
#include "libmesh/fe_interface.h"
#include "metaphysicl/dualnumberarray.h"
#include "Eigen/Dense"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

using MetaPhysicL::NumberArray;

typedef DualNumber<Real, NumberArray<2, Real>> Dual2;

namespace Moose
{
namespace Mortar
{
namespace
{
// These cutoffs identify degeneracy in normalized coefficients and Jacobians above roundoff.
constexpr Real coefficient_tolerance = 1e-14;
constexpr Real jacobian_tolerance = 1e-12;
// Root and residual tolerances preserve ten-digit normalized inverse consistency.
constexpr Real root_tolerance = 1e-10;
constexpr Real inverse_residual_tolerance = 1e-10;
// This matches the reference-space tolerance used when clipping mortar segments.
constexpr Real mortar_reference_tolerance = 1e-8;

bool
isFinite(const Point & point)
{
  return std::isfinite(point(0)) && std::isfinite(point(1)) && std::isfinite(point(2));
}

Real
cross2D(const Point & first, const Point & second)
{
  return first(0) * second(1) - first(1) * second(0);
}

struct PolynomialRoots
{
  std::array<Real, 2> values = {};
  unsigned int count = 0;
};

struct BilinearMap
{
  Point center;
  Point xi;
  Point eta;
  Point mixed;
  Real scale = 0;
};
}

std::vector<unsigned int>
getMortarSubElementNodeIndices(const Elem & parent_elem, const unsigned int sub_elem)
{
  if (sub_elem >= parent_elem.n_sub_elem())
    mooseError("Invalid 3D mortar sub-element index ",
               sub_elem,
               " for parent element ",
               parent_elem.id(),
               " of type ",
               libMesh::Utility::enum_to_string<ElemType>(parent_elem.type()),
               ", which has ",
               parent_elem.n_sub_elem(),
               " sub-elements.");

  switch (parent_elem.type())
  {
    case TRI3:
      return {0, 1, 2};
    case QUAD4:
      return {0, 1, 2, 3};
    case TRI6:
    case TRI7:
      switch (sub_elem)
      {
        case 0:
          return {0, 3, 5};
        case 1:
          return {3, 4, 5};
        case 2:
          return {3, 1, 4};
        case 3:
          return {5, 4, 2};
        default:
          mooseError("Invalid 3D mortar triangular sub-element index ", sub_elem, ".");
      }
    case QUAD8:
      switch (sub_elem)
      {
        case 0:
          return {0, 4, 7};
        case 1:
          return {4, 1, 5};
        case 2:
          return {5, 2, 6};
        case 3:
          return {7, 6, 3};
        case 4:
          return {4, 5, 6, 7};
        default:
          mooseError("Invalid 3D mortar QUAD8 sub-element index ", sub_elem, ".");
      }
    case QUAD9:
      switch (sub_elem)
      {
        case 0:
          return {0, 4, 8, 7};
        case 1:
          return {4, 1, 5, 8};
        case 2:
          return {8, 5, 2, 6};
        case 3:
          return {7, 8, 6, 3};
        default:
          mooseError("Invalid 3D mortar QUAD9 sub-element index ", sub_elem, ".");
      }
    default:
      mooseError("Parent face element ",
                 parent_elem.id(),
                 " has unsupported type ",
                 libMesh::Utility::enum_to_string<ElemType>(parent_elem.type()),
                 " for 3D mortar sub-element topology.");
  }
}

namespace
{
ElemType
subElementType(const ElemType parent_type, const unsigned int sub_elem)
{
  switch (parent_type)
  {
    case TRI3:
    case TRI6:
    case TRI7:
      return TRI3;
    case QUAD4:
    case QUAD9:
      return QUAD4;
    case QUAD8:
      return sub_elem == 4 ? QUAD4 : TRI3;
    default:
      mooseError("Unsupported parent face type ",
                 libMesh::Utility::enum_to_string<ElemType>(parent_type),
                 " for 3D mortar projection.");
  }
}

Real
quadrilateralReferenceViolation(const Point & point)
{
  return std::max({Real(0), -1 - point(0), point(0) - 1, -1 - point(1), point(1) - 1});
}

[[noreturn]] void
projectionFailure(const Elem & msm_elem,
                  const Elem & parent_elem,
                  const unsigned int sub_elem,
                  const unsigned int qp,
                  const std::string & reason)
{
  mooseException("Unable to map 3D mortar quadrature point ",
                 qp,
                 " from mortar segment ",
                 msm_elem.id(),
                 " to subpatch ",
                 sub_elem,
                 " of parent element ",
                 parent_elem.id(),
                 " (",
                 libMesh::Utility::enum_to_string<ElemType>(parent_elem.type()),
                 "): ",
                 reason);
}

PolynomialRoots
realPolynomialRoots(const Real quadratic, const Real linear, const Real constant)
{
  PolynomialRoots roots;
  const Real scale = std::max({std::abs(quadratic), std::abs(linear), std::abs(constant)});
  if (scale == 0)
    return roots;

  const Real a = quadratic / scale;
  const Real b = linear / scale;
  const Real c = constant / scale;
  if (std::abs(a) <= coefficient_tolerance)
  {
    if (std::abs(b) <= coefficient_tolerance)
      return roots;
    roots.values[roots.count++] = -c / b;
    return roots;
  }

  Real discriminant = b * b - 4 * a * c;
  const Real discriminant_scale = b * b + std::abs(4 * a * c);
  if (discriminant < -coefficient_tolerance * std::max(discriminant_scale, Real(1)))
    return roots;
  discriminant = std::max(discriminant, Real(0));

  const Real sqrt_discriminant = std::sqrt(discriminant);
  // Avoid cancellation in one root and recover the other through Vieta's relation.
  const Real q = -0.5 * (b + std::copysign(sqrt_discriminant, b));
  if (std::abs(q) <= coefficient_tolerance)
  {
    roots.values[roots.count++] = -b / (2 * a);
    return roots;
  }

  const Real first_root = q / a;
  const Real second_root = c / q;
  roots.values[roots.count++] = first_root;
  if (std::abs(first_root - second_root) <= root_tolerance)
    return roots;
  roots.values[roots.count++] = second_root;
  return roots;
}

Point
evaluateBilinear(const BilinearMap & map, const Point & target, const Real xi, const Real eta)
{
  return map.center - target + xi * map.xi + eta * map.eta + xi * eta * map.mixed;
}

template <std::size_t N>
void
projectToNormalizedPlane(const Elem & msm_elem,
                         const Elem & parent_elem,
                         const std::vector<unsigned int> & sub_elem_node_indices,
                         const Point & normal,
                         const Point & target,
                         const unsigned int sub_elem,
                         const unsigned int qp,
                         const char * const sub_elem_name,
                         std::array<Point, N> & projected_nodes,
                         Point & projected_target)
{
  mooseAssert(sub_elem_node_indices.size() == N, "Unexpected mortar subpatch node count.");

  Point longest_projected_edge;
  Real length_scale = 0;
  for (const auto first : index_range(sub_elem_node_indices))
  {
    const auto second = (first + 1) % sub_elem_node_indices.size();
    const Point edge = parent_elem.point(sub_elem_node_indices[second]) -
                       parent_elem.point(sub_elem_node_indices[first]);
    const Point projected_edge = edge - (edge * normal) * normal;
    if (projected_edge.norm() > length_scale)
    {
      length_scale = projected_edge.norm();
      longest_projected_edge = projected_edge;
    }
  }

  if (!std::isfinite(length_scale) || length_scale == 0)
    projectionFailure(msm_elem,
                      parent_elem,
                      sub_elem,
                      qp,
                      std::string("the projected ") + sub_elem_name + " is singular");

  const Point first_tangent = longest_projected_edge / length_scale;
  const Point second_tangent = normal.cross(first_tangent).unit();
  const Point origin = parent_elem.point(sub_elem_node_indices[0]);

  for (const auto node : index_range(sub_elem_node_indices))
  {
    const Point offset = parent_elem.point(sub_elem_node_indices[node]) - origin;
    projected_nodes[node] =
        Point((offset * first_tangent) / length_scale, (offset * second_tangent) / length_scale);
  }
  const Point target_offset = target - origin;
  projected_target = Point((target_offset * first_tangent) / length_scale,
                           (target_offset * second_tangent) / length_scale);
}

Point
analyticalTriangleInverse(const Elem & msm_elem,
                          const Elem & parent_elem,
                          const std::vector<unsigned int> & sub_elem_node_indices,
                          const Point & normal,
                          const Point & target,
                          const unsigned int sub_elem,
                          const unsigned int qp)
{
  std::array<Point, 3> projected_nodes;
  Point projected_target;
  projectToNormalizedPlane(msm_elem,
                           parent_elem,
                           sub_elem_node_indices,
                           normal,
                           target,
                           sub_elem,
                           qp,
                           "TRI3",
                           projected_nodes,
                           projected_target);

  const Point first_edge = projected_nodes[1] - projected_nodes[0];
  const Point second_edge = projected_nodes[2] - projected_nodes[0];
  const Real determinant = cross2D(first_edge, second_edge);
  if (std::abs(determinant) <= jacobian_tolerance)
    projectionFailure(msm_elem, parent_elem, sub_elem, qp, "the projected TRI3 is singular");

  const Point right_hand_side = projected_target - projected_nodes[0];
  const Real xi = cross2D(right_hand_side, second_edge) / determinant;
  const Real eta = cross2D(first_edge, right_hand_side) / determinant;
  if (!std::isfinite(xi) || !std::isfinite(eta))
    projectionFailure(msm_elem, parent_elem, sub_elem, qp, "the TRI3 inverse is not finite");

  const Point unsnapped_result(xi, eta);
  const Point unsnapped_residual =
      projected_nodes[0] + xi * first_edge + eta * second_edge - projected_target;
  if (unsnapped_residual.norm() > inverse_residual_tolerance)
    projectionFailure(
        msm_elem, parent_elem, sub_elem, qp, "the TRI3 inverse does not satisfy the projection");

  std::array<Real, 3> barycentric = {{1 - xi - eta, xi, eta}};
  const Real violation = std::max({Real(0), -barycentric[0], -barycentric[1], -barycentric[2]});
  if (violation == 0)
    return unsnapped_result;
  if (violation > mortar_reference_tolerance)
    projectionFailure(
        msm_elem, parent_elem, sub_elem, qp, "the TRI3 inverse is outside the subpatch");

  // A clipping-sized violation is roundoff at an edge: clamp all barycentric coordinates and
  // renormalize to preserve their partition of unity.
  for (auto & coordinate : barycentric)
    coordinate = std::clamp(coordinate, Real(0), Real(1));
  const Real barycentric_sum = barycentric[0] + barycentric[1] + barycentric[2];
  for (auto & coordinate : barycentric)
    coordinate /= barycentric_sum;

  const Point result(barycentric[1], barycentric[2]);
  const Point snapped_residual =
      projected_nodes[0] + result(0) * first_edge + result(1) * second_edge - projected_target;
  if (snapped_residual.norm() > mortar_reference_tolerance)
    projectionFailure(msm_elem,
                      parent_elem,
                      sub_elem,
                      qp,
                      "the snapped TRI3 inverse does not satisfy the projection");
  return result;
}

BilinearMap
prepareQuadrilateralMap(const std::array<Point, 4> & points,
                        const Elem & msm_elem,
                        const Elem & parent_elem,
                        const unsigned int sub_elem,
                        const unsigned int qp)
{
  BilinearMap map;
  map.center = 0.25 * (points[0] + points[1] + points[2] + points[3]);
  map.xi = 0.25 * (-points[0] + points[1] + points[2] - points[3]);
  map.eta = 0.25 * (-points[0] - points[1] + points[2] + points[3]);
  map.mixed = 0.25 * (points[0] - points[1] + points[2] - points[3]);
  map.scale = std::max({map.xi.norm(), map.eta.norm(), map.mixed.norm()});
  if (!std::isfinite(map.scale) || map.scale == 0)
    projectionFailure(
        msm_elem, parent_elem, sub_elem, qp, "the projected QUAD4 has invalid coefficients");

  // The bilinear Jacobian is affine, so nonzero corner determinants with one sign exclude folding.
  Real orientation = 0;
  for (const auto xi : {-1.0, 1.0})
    for (const auto eta : {-1.0, 1.0})
    {
      const Real determinant = cross2D(map.xi + eta * map.mixed, map.eta + xi * map.mixed);
      if (std::abs(determinant) <= jacobian_tolerance)
        projectionFailure(msm_elem, parent_elem, sub_elem, qp, "the projected QUAD4 is singular");
      if (orientation == 0)
        orientation = std::copysign(1.0, determinant);
      else if (orientation * determinant < 0)
        projectionFailure(msm_elem, parent_elem, sub_elem, qp, "the projected QUAD4 is folded");
    }
  return map;
}

Point
inverseMapQuadrilateral(const BilinearMap & map,
                        const Point & target,
                        const Elem & msm_elem,
                        const Elem & parent_elem,
                        const unsigned int sub_elem,
                        const unsigned int qp)
{
  std::vector<Point> strict_candidates;
  std::vector<Point> tolerance_candidates;

  auto store_candidate = [](const Point & candidate, auto & candidates)
  {
    if (std::none_of(candidates.begin(),
                     candidates.end(),
                     [&candidate](const Point & existing)
                     { return (candidate - existing).norm() <= root_tolerance; }))
      candidates.push_back(candidate);
  };

  auto add_candidate = [&](const Real xi, const Real eta)
  {
    Point candidate(xi, eta);
    if (!isFinite(candidate))
      return;

    if (evaluateBilinear(map, target, xi, eta).norm() > inverse_residual_tolerance)
      return;

    const Real violation = quadrilateralReferenceViolation(candidate);
    if (violation == 0)
    {
      store_candidate(candidate, strict_candidates);
      return;
    }
    if (violation > mortar_reference_tolerance)
      return;

    // Only tolerance-sized exterior roots may be snapped, and the snapped point must still
    // satisfy the normalized projection equation.
    const Point unsnapped_candidate = candidate;
    candidate(0) = std::clamp(candidate(0), Real(-1), Real(1));
    candidate(1) = std::clamp(candidate(1), Real(-1), Real(1));
    if (evaluateBilinear(map, target, candidate(0), candidate(1)).norm() >
        mortar_reference_tolerance)
      return;
    store_candidate(unsnapped_candidate, tolerance_candidates);
  };

  // Enumerate both eliminations because one reconstruction direction can be singular at a root.
  const auto xi_roots =
      realPolynomialRoots(cross2D(map.xi, map.mixed),
                          cross2D(map.center - target, map.mixed) + cross2D(map.xi, map.eta),
                          cross2D(map.center - target, map.eta));
  const Real direction_tolerance_sq =
      coefficient_tolerance * coefficient_tolerance * map.scale * map.scale;
  for (const auto root : make_range(xi_roots.count))
  {
    const Real xi = xi_roots.values[root];
    const Point eta_direction = map.eta + xi * map.mixed;
    const Real denominator = eta_direction.norm_sq();
    if (denominator > direction_tolerance_sq)
      add_candidate(xi, (((target - map.center) - xi * map.xi) * eta_direction) / denominator);
  }

  const auto eta_roots =
      realPolynomialRoots(cross2D(map.eta, map.mixed),
                          cross2D(map.center - target, map.mixed) + cross2D(map.eta, map.xi),
                          cross2D(map.center - target, map.xi));
  for (const auto root : make_range(eta_roots.count))
  {
    const Real eta = eta_roots.values[root];
    const Point xi_direction = map.xi + eta * map.mixed;
    const Real denominator = xi_direction.norm_sq();
    if (denominator > direction_tolerance_sq)
      add_candidate((((target - map.center) - eta * map.eta) * xi_direction) / denominator, eta);
  }

  // A true in-domain root takes precedence over a clipping-tolerance boundary candidate.
  if (strict_candidates.size() == 1)
    return strict_candidates[0];
  if (strict_candidates.empty() && tolerance_candidates.size() == 1)
  {
    auto candidate = tolerance_candidates[0];
    candidate(0) = std::clamp(candidate(0), Real(-1), Real(1));
    candidate(1) = std::clamp(candidate(1), Real(-1), Real(1));
    return candidate;
  }

  projectionFailure(msm_elem,
                    parent_elem,
                    sub_elem,
                    qp,
                    "the analytical fallback did not find one unique in-domain QUAD4 inverse");
}

Point
analyticalQuadrilateralInverse(const Elem & msm_elem,
                               const Elem & parent_elem,
                               const std::vector<unsigned int> & sub_elem_node_indices,
                               const Point & normal,
                               const Point & target,
                               const unsigned int sub_elem,
                               const unsigned int qp)
{
  std::array<Point, 4> projected_nodes;
  Point projected_target;
  projectToNormalizedPlane(msm_elem,
                           parent_elem,
                           sub_elem_node_indices,
                           normal,
                           target,
                           sub_elem,
                           qp,
                           "QUAD4",
                           projected_nodes,
                           projected_target);

  return inverseMapQuadrilateral(
      prepareQuadrilateralMap(projected_nodes, msm_elem, parent_elem, sub_elem, qp),
      projected_target,
      msm_elem,
      parent_elem,
      sub_elem,
      qp);
}
}

void
mapQPoints3dFromReference(const Elem & mortar_segment_elem,
                          const MortarSegmentReferencePoints & reference_points,
                          const QBase & qrule_msm,
                          std::vector<Point> & secondary_q_pts,
                          std::vector<Point> & primary_q_pts)
{
  mooseAssert(mortar_segment_elem.type() == TRI3,
              "Reference interpolation expects triangular mortar segments.");
  const FEType fe_type(FIRST, LAGRANGE);

  for (const auto qp : make_range(qrule_msm.n_points()))
  {
    Point secondary_qp;
    Point primary_qp;

    for (const auto n : index_range(reference_points.secondary_reference_points))
    {
      const auto phi =
          FEInterface::shape(fe_type, &mortar_segment_elem, n, qrule_msm.qp(qp), false);
      secondary_qp += phi * reference_points.secondary_reference_points[n];
      primary_qp += phi * reference_points.primary_reference_points[n];
    }

    secondary_q_pts.push_back(secondary_qp);
    primary_q_pts.push_back(primary_qp);
  }
}

void
projectQPoints3d(const Elem * const msm_elem,
                 const Elem * const primal_elem,
                 const unsigned int sub_elem_index,
                 const QBase & qrule_msm,
                 std::vector<Point> & q_pts)
{
  const auto msm_elem_order = msm_elem->default_order();
  const auto msm_elem_type = msm_elem->type();

  // Get normal to linearized element, could store and query but computation is easy
  const Point e1 = msm_elem->point(0) - msm_elem->point(1);
  const Point e2 = msm_elem->point(2) - msm_elem->point(1);
  const Point normal = e2.cross(e1).unit();

  // Get sub-elem (for second order meshes, otherwise trivial)
  const auto sub_elem = msm_elem->get_extra_integer(sub_elem_index);
  const ElemType primal_type = primal_elem->type();
  const ElemType sub_elem_type = subElementType(primal_type, sub_elem);

  // Transforms quadrature point from first order sub-elements (in case of second-order)
  // to primal element
  auto transform_qp = [primal_type, sub_elem](const Real nu, const Real xi)
  {
    switch (primal_type)
    {
      case TRI3:
        return Point(nu, xi, 0);
      case QUAD4:
        return Point(nu, xi, 0);
      case TRI6:
      case TRI7:
        switch (sub_elem)
        {
          case 0:
            return Point(0.5 * nu, 0.5 * xi, 0);
          case 1:
            return Point(0.5 * (1 - xi), 0.5 * (nu + xi), 0);
          case 2:
            return Point(0.5 * (1 + nu), 0.5 * xi, 0);
          case 3:
            return Point(0.5 * nu, 0.5 * (1 + xi), 0);
          default:
            mooseError("get_sub_elem_indices: Invalid sub_elem: ", sub_elem);
        }
      case QUAD8:
        switch (sub_elem)
        {
          case 0:
            return Point(nu - 1, xi - 1, 0);
          case 1:
            return Point(nu + xi, xi - 1, 0);
          case 2:
            return Point(1 - xi, nu + xi, 0);
          case 3:
            return Point(nu - 1, nu + xi, 0);
          case 4:
            return Point(0.5 * (nu - xi), 0.5 * (nu + xi), 0);
          default:
            mooseError("get_sub_elem_indices: Invalid sub_elem: ", sub_elem);
        }
      case QUAD9:
        switch (sub_elem)
        {
          case 0:
            return Point(0.5 * (nu - 1), 0.5 * (xi - 1), 0);
          case 1:
            return Point(0.5 * (nu + 1), 0.5 * (xi - 1), 0);
          case 2:
            return Point(0.5 * (nu + 1), 0.5 * (xi + 1), 0);
          case 3:
            return Point(0.5 * (nu - 1), 0.5 * (xi + 1), 0);
          default:
            mooseError("get_sub_elem_indices: Invalid sub_elem: ", sub_elem);
        }
      default:
        mooseError("transform_qp: Face element type: ",
                   libMesh::Utility::enum_to_string<ElemType>(primal_type),
                   " invalid for 3D mortar");
    }
  };

  // Get sub-elem node indices
  const auto sub_elem_node_indices = getMortarSubElementNodeIndices(*primal_elem, sub_elem);

  // Loop through quadrature points on msm_elem
  for (auto qp : make_range(qrule_msm.n_points()))
  {
    // Get physical point on msm_elem to project
    Point x0;
    for (auto n : make_range(msm_elem->n_nodes()))
      x0 += Moose::fe_lagrange_2D_shape(msm_elem_type,
                                        msm_elem_order,
                                        n,
                                        static_cast<const TypeVector<Real> &>(qrule_msm.qp(qp))) *
            msm_elem->point(n);

    if (sub_elem_type == TRI3)
    {
      const Point sub_elem_point = analyticalTriangleInverse(
          *msm_elem, *primal_elem, sub_elem_node_indices, normal, x0, sub_elem, qp);
      const Point parent_point = transform_qp(sub_elem_point(0), sub_elem_point(1));
      if (!isFinite(parent_point) ||
          !primal_elem->on_reference_element(parent_point, mortar_reference_tolerance))
        projectionFailure(*msm_elem,
                          *primal_elem,
                          sub_elem,
                          qp,
                          "the recovered TRI3 point is outside the parent face");
      q_pts.push_back(parent_point);
      continue;
    }

    // Use msm_elem quadrature point as initial guess
    // (will be correct for aligned meshes)
    Dual2 xi1{};
    xi1.value() = qrule_msm.qp(qp)(0);
    xi1.derivatives()[0] = 1.0;
    Dual2 xi2{};
    xi2.value() = qrule_msm.qp(qp)(1);
    xi2.derivatives()[1] = 1.0;
    VectorValue<Dual2> xi(xi1, xi2, 0);
    unsigned int current_iterate = 0, max_iterates = 10;

    // Project qp from mortar segments to first order sub-elements (elements in case of first order
    // geometry)
    do
    {
      VectorValue<Dual2> x1;
      for (auto n : make_range(sub_elem_node_indices.size()))
        x1 += Moose::fe_lagrange_2D_shape(sub_elem_type, FIRST, n, xi) *
              primal_elem->point(sub_elem_node_indices[n]);
      auto u = x1 - x0;

      VectorValue<Dual2> F(u(1) * normal(2) - u(2) * normal(1),
                           u(2) * normal(0) - u(0) * normal(2),
                           u(0) * normal(1) - u(1) * normal(0));

      Real projection_tolerance(1e-10);

      // Normalize tolerance with quantities involved in the projection.
      // Absolute projection tolerance is loosened for displacements larger than those on the order
      // of one. Tightening the tolerance for displacements of smaller orders causes this tolerance
      // to not be reached in a number of tests.
      if (!u.is_zero() && u.norm().value() > 1.0)
        projection_tolerance *= u.norm().value();

      if (MetaPhysicL::raw_value(F).norm() < projection_tolerance)
        break;

      RealEigenMatrix J(3, 2);
      J << F(0).derivatives()[0], F(0).derivatives()[1], F(1).derivatives()[0],
          F(1).derivatives()[1], F(2).derivatives()[0], F(2).derivatives()[1];
      RealEigenVector f(3);
      f << F(0).value(), F(1).value(), F(2).value();
      const RealEigenVector dxi = -J.colPivHouseholderQr().solve(f);

      xi(0) += dxi(0);
      xi(1) += dxi(1);
    } while (++current_iterate < max_iterates);

    const Point newton_sub_elem_point(xi(0).value(), xi(1).value());
    const Point newton_parent_point =
        transform_qp(newton_sub_elem_point(0), newton_sub_elem_point(1));
    const bool newton_point_is_valid =
        current_iterate < max_iterates && isFinite(newton_sub_elem_point) &&
        isFinite(newton_parent_point) &&
        quadrilateralReferenceViolation(newton_sub_elem_point) == 0 &&
        primal_elem->on_reference_element(newton_parent_point, mortar_reference_tolerance);

    if (newton_point_is_valid)
    {
      q_pts.push_back(newton_parent_point);
      continue;
    }

    if (sub_elem_type == QUAD4)
    {
      // Newton can converge to the exterior root of a distorted bilinear QUAD.
      const Point fallback_point = analyticalQuadrilateralInverse(
          *msm_elem, *primal_elem, sub_elem_node_indices, normal, x0, sub_elem, qp);
      const Point parent_point = transform_qp(fallback_point(0), fallback_point(1));
      if (!isFinite(parent_point) ||
          !primal_elem->on_reference_element(parent_point, mortar_reference_tolerance))
        projectionFailure(*msm_elem,
                          *primal_elem,
                          sub_elem,
                          qp,
                          "the recovered point is outside the parent face");
      q_pts.push_back(parent_point);
      continue;
    }

    if (current_iterate == max_iterates)
      mooseError("Newton iteration for mortar quadrature mapping msm element: ",
                 msm_elem->id(),
                 " to elem: ",
                 primal_elem->id(),
                 " didn't converge. MSM element volume: ",
                 msm_elem->volume());

    projectionFailure(
        *msm_elem, *primal_elem, sub_elem, qp, "the Newton result is outside the parent face");
  }
}
}
}
