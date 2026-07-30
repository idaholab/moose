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

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace Moose
{
namespace Mortar
{
namespace
{
// These cutoffs identify degeneracy in normalized polynomial coefficients and Jacobians above
// floating-point roundoff.
constexpr Real coefficient_tolerance = 1e-14;
constexpr Real jacobian_tolerance = 1e-12;
// Root deduplication and projection validation preserve ten-digit normalized inverse consistency.
constexpr Real root_tolerance = 1e-10;
constexpr Real projection_tolerance = 1e-10;
// This matches the reference-space tolerance used when clipping mortar segments.
constexpr Real minimum_reference_tolerance = 1e-8;

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
  bool indeterminate = false;
};

struct BilinearMap
{
  Point center;
  Point xi;
  Point eta;
  Point mixed;
  Real scale = 0;
  bool affine = false;
};

Point
evaluateBilinear(const BilinearMap & map,
                 const Point & translated_center,
                 const Real xi,
                 const Real eta)
{
  return translated_center + xi * map.xi + eta * map.eta + xi * eta * map.mixed;
}

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

[[noreturn]] void
projectionGeometryFailure(const Elem & msm_elem,
                          const Elem & parent_elem,
                          const unsigned int sub_elem,
                          const std::string & reason)
{
  mooseException("Unable to prepare 3D mortar projection from mortar segment ",
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
  {
    roots.indeterminate = true;
    return roots;
  }

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

Real
quadReferenceViolation(const Point & point)
{
  return std::max({Real(0), -1 - point(0), point(0) - 1, -1 - point(1), point(1) - 1});
}

Point
inverseMapTriangle(const std::array<Point, 4> & points,
                   const Elem & msm_elem,
                   const Elem & parent_elem,
                   const unsigned int sub_elem,
                   const unsigned int qp,
                   const Real clipping_tolerance)
{
  const Point first_edge = points[1] - points[0];
  const Point second_edge = points[2] - points[0];
  const Real determinant = cross2D(first_edge, second_edge);
  if (std::abs(determinant) <= jacobian_tolerance)
    projectionFailure(msm_elem, parent_elem, sub_elem, qp, "the projected TRI3 is singular");

  const Point right_hand_side = -points[0];
  const Real xi = cross2D(right_hand_side, second_edge) / determinant;
  const Real eta = cross2D(first_edge, right_hand_side) / determinant;
  if (!std::isfinite(xi) || !std::isfinite(eta))
    projectionFailure(msm_elem, parent_elem, sub_elem, qp, "the TRI3 inverse is not finite");

  const Point unsnapped_result(xi, eta);
  const Point unsnapped_residual =
      points[0] + unsnapped_result(0) * first_edge + unsnapped_result(1) * second_edge;
  if (unsnapped_residual.norm() > projection_tolerance)
    projectionFailure(
        msm_elem, parent_elem, sub_elem, qp, "the TRI3 inverse does not satisfy the projection");

  std::array<Real, 3> barycentric = {{1 - xi - eta, xi, eta}};
  const Real violation = std::max({Real(0), -barycentric[0], -barycentric[1], -barycentric[2]});
  if (violation == 0)
    return unsnapped_result;

  for (auto & coordinate : barycentric)
    coordinate = std::clamp(coordinate, Real(0), Real(1));
  const Real sum = barycentric[0] + barycentric[1] + barycentric[2];
  for (auto & coordinate : barycentric)
    coordinate /= sum;

  const Point result(barycentric[1], barycentric[2]);
  const Point residual = points[0] + result(0) * first_edge + result(1) * second_edge;
  if (residual.norm() > clipping_tolerance)
    projectionFailure(msm_elem,
                      parent_elem,
                      sub_elem,
                      qp,
                      "the validated TRI3 inverse does not satisfy the projection equation");
  return result;
}

BilinearMap
prepareQuadrilateralMap(const std::array<Point, 4> & points,
                        const Elem & msm_elem,
                        const Elem & parent_elem,
                        const unsigned int sub_elem)
{
  BilinearMap map;
  map.center = 0.25 * (points[0] + points[1] + points[2] + points[3]);
  map.xi = 0.25 * (-points[0] + points[1] + points[2] - points[3]);
  map.eta = 0.25 * (-points[0] - points[1] + points[2] + points[3]);
  map.mixed = 0.25 * (points[0] - points[1] + points[2] - points[3]);
  map.scale = std::max({map.xi.norm(), map.eta.norm(), map.mixed.norm()});
  if (!std::isfinite(map.scale) || map.scale == 0)
    projectionGeometryFailure(
        msm_elem, parent_elem, sub_elem, "the projected QUAD4 has invalid coefficients");
  map.affine = map.mixed.norm() <= coefficient_tolerance * map.scale;

  // The bilinear Jacobian is affine, so nonzero corner determinants with one sign exclude folding.
  Real orientation = 0;
  for (const auto xi : {-1.0, 1.0})
    for (const auto eta : {-1.0, 1.0})
    {
      const Real determinant = cross2D(map.xi + eta * map.mixed, map.eta + xi * map.mixed);
      if (std::abs(determinant) <= jacobian_tolerance)
        projectionGeometryFailure(
            msm_elem, parent_elem, sub_elem, "the projected QUAD4 is singular");
      if (orientation == 0)
        orientation = std::copysign(1.0, determinant);
      else if (orientation * determinant < 0)
        projectionGeometryFailure(msm_elem, parent_elem, sub_elem, "the projected QUAD4 is folded");
    }
  return map;
}

Point
inverseMapQuadrilateral(const BilinearMap & map,
                        const Point & target,
                        const Elem & msm_elem,
                        const Elem & parent_elem,
                        const unsigned int sub_elem,
                        const unsigned int qp,
                        const Real clipping_tolerance)
{
  const Point translated_center = map.center - target;
  std::array<Point, 2> candidates;
  std::array<Point, 2> snapped_candidates;
  unsigned int candidate_count = 0;
  unsigned int snapped_candidate_count = 0;
  bool use_reverse_elimination = false;
  bool too_many_candidates = false;
  bool too_many_snapped_candidates = false;
  auto store_candidate = [&](const Point & candidate,
                             auto & stored_candidates,
                             auto & stored_candidate_count,
                             auto & overflow)
  {
    for (const auto existing_candidate : make_range(stored_candidate_count))
      if ((candidate - stored_candidates[existing_candidate]).norm() <= root_tolerance)
        return;
    if (stored_candidate_count == stored_candidates.size())
    {
      overflow = true;
      return;
    }
    stored_candidates[stored_candidate_count++] = candidate;
  };
  auto add_candidate = [&](const Real xi, const Real eta)
  {
    Point candidate(xi, eta);
    if (!isFinite(candidate))
    {
      use_reverse_elimination = true;
      return;
    }
    if (evaluateBilinear(map, translated_center, xi, eta).norm() > projection_tolerance)
    {
      use_reverse_elimination = true;
      return;
    }
    const Real violation = quadReferenceViolation(candidate);
    if (violation > 0)
    {
      // Permit snapping only when the boundary point still satisfies the clipping tolerance.
      candidate(0) = std::clamp(candidate(0), Real(-1), Real(1));
      candidate(1) = std::clamp(candidate(1), Real(-1), Real(1));
      if (evaluateBilinear(map, translated_center, candidate(0), candidate(1)).norm() >
          clipping_tolerance)
        return;
      store_candidate(
          candidate, snapped_candidates, snapped_candidate_count, too_many_snapped_candidates);
    }
    else
      store_candidate(candidate, candidates, candidate_count, too_many_candidates);
  };

  if (map.affine)
  {
    const Point right_hand_side = -translated_center;
    const Real determinant = cross2D(map.xi, map.eta);
    add_candidate(cross2D(right_hand_side, map.eta) / determinant,
                  cross2D(map.xi, right_hand_side) / determinant);
  }
  else
  {
    // Eliminate eta first; reverse the elimination if that form is indeterminate or cannot
    // reconstruct and validate every root.
    const auto xi_roots =
        realPolynomialRoots(cross2D(map.xi, map.mixed),
                            cross2D(translated_center, map.mixed) + cross2D(map.xi, map.eta),
                            cross2D(translated_center, map.eta));
    use_reverse_elimination = xi_roots.indeterminate;
    const Real direction_tolerance_sq =
        coefficient_tolerance * coefficient_tolerance * map.scale * map.scale;
    for (const auto root : make_range(xi_roots.count))
    {
      const Real xi = xi_roots.values[root];
      const Point eta_direction = map.eta + xi * map.mixed;
      const Real denominator = eta_direction.norm_sq();
      if (denominator > direction_tolerance_sq)
        add_candidate(xi, ((-translated_center - xi * map.xi) * eta_direction) / denominator);
      else
        use_reverse_elimination = true;
    }
    if (use_reverse_elimination)
    {
      const auto eta_roots =
          realPolynomialRoots(cross2D(map.eta, map.mixed),
                              cross2D(translated_center, map.mixed) + cross2D(map.eta, map.xi),
                              cross2D(translated_center, map.xi));
      for (const auto root : make_range(eta_roots.count))
      {
        const Real eta = eta_roots.values[root];
        const Point xi_direction = map.xi + eta * map.mixed;
        const Real denominator = xi_direction.norm_sq();
        if (denominator > direction_tolerance_sq)
          add_candidate(((-translated_center - eta * map.eta) * xi_direction) / denominator, eta);
      }
    }
  }

  // Prefer the exact in-domain inverse over a clipping-tolerance candidate.
  if (candidate_count == 1 && !too_many_candidates)
    return candidates[0];

  if (candidate_count == 0 && snapped_candidate_count == 1 && !too_many_snapped_candidates)
    return snapped_candidates[0];

  std::ostringstream message;
  message << "expected one in-domain QUAD4 inverse but found " << candidate_count
          << " feasible and " << snapped_candidate_count << " clipping-tolerance candidates";
  projectionFailure(msm_elem, parent_elem, sub_elem, qp, message.str());
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
  // This compatibility path has no clipping metadata, so reconstruct the segment normal and use
  // the base reference tolerance.
  const Point first_edge = msm_elem->point(0) - msm_elem->point(1);
  const Point second_edge = msm_elem->point(2) - msm_elem->point(1);
  const Point normal = second_edge.cross(first_edge);
  if (!isFinite(normal) || normal.norm() == 0)
    mooseException("Cannot determine a normal for degenerate mortar segment ", msm_elem->id(), ".");

  projectQPoints3d(*msm_elem, *primal_elem, sub_elem_index, normal, 0, qrule_msm, q_pts);
}

void
projectQPoints3d(const Elem & msm_elem,
                 const Elem & primal_elem,
                 const unsigned int sub_elem_index,
                 const Point & projection_normal,
                 const Real clipping_area_tolerance,
                 const QBase & qrule_msm,
                 std::vector<Point> & q_pts)
{
  if (msm_elem.type() != TRI3)
    mooseError("3D mortar quadrature projection requires a TRI3 mortar segment, but segment ",
               msm_elem.id(),
               " has type ",
               libMesh::Utility::enum_to_string<ElemType>(msm_elem.type()),
               ".");

  const auto sub_elem = msm_elem.get_extra_integer(sub_elem_index);
  const auto sub_elem_node_indices = getMortarSubElementNodeIndices(primal_elem, sub_elem);
  const auto sub_elem_type = subElementType(primal_elem.type(), sub_elem);

  if (!isFinite(projection_normal))
    mooseException("Invalid projection-plane normal for mortar segment ", msm_elem.id(), ".");
  const Real normal_norm = projection_normal.norm();
  if (!std::isfinite(normal_norm) || normal_norm == 0)
    mooseException("Invalid projection-plane normal for mortar segment ", msm_elem.id(), ".");
  if (!std::isfinite(clipping_area_tolerance) || clipping_area_tolerance < 0)
    mooseError("Invalid clipping area tolerance for mortar segment ", msm_elem.id(), ".");
  const Point normal = projection_normal / normal_norm;

  // Express the subpatch in a normalized clipping-plane basis before analytical inversion, making
  // tolerances independent of translation and element scale.
  Point longest_projected_edge;
  Real length_scale = 0;
  Real minimum_edge_length = std::numeric_limits<Real>::max();
  for (const auto first : index_range(sub_elem_node_indices))
  {
    const auto second = (first + 1) % sub_elem_node_indices.size();
    const Point edge = primal_elem.point(sub_elem_node_indices[second]) -
                       primal_elem.point(sub_elem_node_indices[first]);
    const Point projected_edge = edge - (edge * normal) * normal;
    const Real projected_edge_length = projected_edge.norm();
    minimum_edge_length = std::min(minimum_edge_length, projected_edge_length);
    if (projected_edge_length > length_scale)
    {
      length_scale = projected_edge_length;
      longest_projected_edge = projected_edge;
    }
  }

  if (!std::isfinite(length_scale) || !std::isfinite(minimum_edge_length) || length_scale == 0 ||
      minimum_edge_length == 0)
    mooseException("Parent subpatch ",
                   sub_elem,
                   " of element ",
                   primal_elem.id(),
                   " is singular when projected along the mortar clipping normal.");

  // Convert the helper's physical area tolerance to the normalized projection-residual scale.
  const Real clipping_tolerance = std::max(
      minimum_reference_tolerance, clipping_area_tolerance / (minimum_edge_length * length_scale));
  const Point first_tangent = longest_projected_edge / length_scale;
  const Point second_tangent = normal.cross(first_tangent).unit();
  const Point origin = primal_elem.point(sub_elem_node_indices[0]);

  std::array<Point, 4> projected_nodes = {};
  for (const auto node : index_range(sub_elem_node_indices))
  {
    const Point offset = primal_elem.point(sub_elem_node_indices[node]) - origin;
    projected_nodes[node] =
        Point((offset * first_tangent) / length_scale, (offset * second_tangent) / length_scale);
  }

  const BilinearMap quadrilateral_map =
      sub_elem_type == QUAD4
          ? prepareQuadrilateralMap(projected_nodes, msm_elem, primal_elem, sub_elem)
          : BilinearMap();

  for (const auto qp : make_range(qrule_msm.n_points()))
  {
    const Point mortar_reference_point = qrule_msm.qp(qp);
    if (!isFinite(mortar_reference_point))
      mooseError("Mortar segment ",
                 msm_elem.id(),
                 " has a non-finite quadrature point at index ",
                 qp,
                 ".");

    Point physical_mortar_point;
    for (const auto node : make_range(msm_elem.n_nodes()))
      physical_mortar_point +=
          Moose::fe_lagrange_2D_shape(TRI3, FIRST, node, mortar_reference_point) *
          msm_elem.point(node);

    const Point target_offset = physical_mortar_point - origin;
    const Point projected_target((target_offset * first_tangent) / length_scale,
                                 (target_offset * second_tangent) / length_scale);
    Point sub_elem_reference_point;
    if (sub_elem_type == TRI3)
    {
      auto translated_nodes = projected_nodes;
      for (const auto node : make_range(sub_elem_node_indices.size()))
        translated_nodes[node] -= projected_target;
      sub_elem_reference_point = inverseMapTriangle(
          translated_nodes, msm_elem, primal_elem, sub_elem, qp, clipping_tolerance);
    }
    else
      sub_elem_reference_point = inverseMapQuadrilateral(quadrilateral_map,
                                                         projected_target,
                                                         msm_elem,
                                                         primal_elem,
                                                         sub_elem,
                                                         qp,
                                                         clipping_tolerance);

    // Map the first-order subpatch coordinate through its vertices in the parent reference domain.
    Point parent_reference_point;
    for (const auto node : index_range(sub_elem_node_indices))
      parent_reference_point +=
          Moose::fe_lagrange_2D_shape(sub_elem_type, FIRST, node, sub_elem_reference_point) *
          primal_elem.master_point(sub_elem_node_indices[node]);

    if (!isFinite(parent_reference_point) ||
        !primal_elem.on_reference_element(parent_reference_point, minimum_reference_tolerance))
    {
      std::ostringstream message;
      message << "the recovered parent reference point " << parent_reference_point
              << " is outside the parent domain";
      projectionFailure(msm_elem, primal_elem, sub_elem, qp, message.str());
    }

    q_pts.push_back(parent_reference_point);
  }
}
}
}
