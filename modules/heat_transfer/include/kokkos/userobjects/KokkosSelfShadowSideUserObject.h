//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosSideUserObject.h"

/**
 * Kokkos user object computing the illumination state of every quadrature point on a sideset
 * irradiated by a parallel beam, accounting for shadowing by other sides of the same sideset.
 *
 * The sides are triangulated (3D) or split into line segments (2D) on the host, rotated so that the
 * beam propagates along +z (3D) or +x (2D), and gathered on all processes. The occlusion test of
 * every quadrature point against all gathered segments or triangles is then performed in the Kokkos
 * side loop. The result is a bitmask per local side with the bit at position qp set if the
 * quadrature point is illuminated.
 */
class KokkosSelfShadowSideUserObject : public Moose::Kokkos::SideUserObject
{
  using Real3 = Moose::Kokkos::Real3;

public:
  static InputParameters validParams();

  KokkosSelfShadowSideUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void compute() override;
  virtual void finalize() override {}

  template <typename Derived>
  KOKKOS_FUNCTION void execute(Datum & datum) const;

  /**
   * Get the illumination bitmask of a local side
   * @param elem The contiguous element ID
   * @param side The side index
   * @returns The bitmask with the bit at position qp set if the quadrature point is illuminated
   */
  /// Whether the user object runs on the displaced mesh
  bool useDisplacedMesh() const { return getParam<bool>("use_displaced_mesh"); }

  /**
   * Get the illumination bitmask of a local side
   * @param elem The contiguous element ID
   * @param side The side index
   * @returns The bitmask with the bit at position qp set if the quadrature point is illuminated
   */
  KOKKOS_FUNCTION unsigned int illumination(ContiguousElementID elem, unsigned int side) const;

  /**
   * Get the illumination bitmasks of the local sides indexed by (side, contiguous element ID). A
   * copy of the returned array shares data with this user object and reflects later updates.
   */
  const Moose::Kokkos::Array2D<unsigned int> & illuminationStatus() const { return _illumination; }

private:
  /// Whether the point in the rotated frame is shaded by a segment of another side (2D)
  KOKKOS_FUNCTION bool check2DIllumination(const Real3 & qp, const dof_id_type id) const;
  /// Whether the point in the rotated frame is shaded by a triangle of another side (3D)
  KOKKOS_FUNCTION bool check3DIllumination(const Real3 & qp, const dof_id_type id) const;

  /// Global key identifying an element side, unique across processes
  dof_id_type sideKey(const Elem & elem, const unsigned int side) const;

  /// Add the rotated line segments (2D) or triangles (3D) of a side to the lists
  void addPrimitives(const Elem & side_elem,
                     const dof_id_type key,
                     std::vector<Real> & points,
                     std::vector<dof_id_type> & keys) const;

  /// problem dimension
  const unsigned int _dim;
  /// raw illumination vector data (direction the radiation is propagating in)
  std::vector<const PostprocessorValue *> _raw_direction;
  /// matrix that rotates the direction onto the x-axis (2D) or the z-axis (3D)
  Moose::Kokkos::Real33 _rotation;
  /// number of vertices per primitive: 2 for line segments and 3 for triangles
  const unsigned int _n_vertices;
  /// maximum number of sides per element, used to build global side keys
  const unsigned int _max_sides;
  /// vertex coordinates of the global primitives in the rotated frame, indexed by (coordinate,
  /// vertex, primitive)
  Moose::Kokkos::Array3D<Real> _primitives;
  /// global side key of each primitive, used to prevent a side from shading itself
  Moose::Kokkos::Array<dof_id_type> _primitive_ids;
  /// global side key of each local side indexed by (side, contiguous element ID)
  Moose::Kokkos::Array2D<dof_id_type> _side_keys;
  /// illumination bitmask of each local side indexed by (side, contiguous element ID)
  Moose::Kokkos::Array2D<unsigned int> & _illumination;
};

KOKKOS_FUNCTION inline unsigned int
KokkosSelfShadowSideUserObject::illumination(ContiguousElementID elem, unsigned int side) const
{
  return _illumination(side, elem);
}

inline dof_id_type
KokkosSelfShadowSideUserObject::sideKey(const Elem & elem, const unsigned int side) const
{
  return elem.id() * _max_sides + side;
}

KOKKOS_FUNCTION inline bool
KokkosSelfShadowSideUserObject::check2DIllumination(const Real3 & qp, const dof_id_type id) const
{
  const auto x = qp(0);
  const auto y = qp(1);

  // loop over all line segments until one is found that provides shade
  for (dof_id_type l = 0; l < _primitive_ids.size(); ++l)
  {
    // make sure a side never shades itself
    if (_primitive_ids[l] == id)
      continue;

    const bool ordered = _primitives(1, 0, l) <= _primitives(1, 1, l);
    const unsigned int lo = ordered ? 0 : 1;
    const unsigned int hi = ordered ? 1 : 0;

    const auto y1 = _primitives(1, lo, l);
    const auto y2 = _primitives(1, hi, l);

    if (y >= y1 && y <= y2)
    {
      // line segment is oriented parallel to the irradiation direction
      if (::Kokkos::abs(y2 - y1) < libMesh::TOLERANCE)
        return false;

      // segment is in line with the QP in radiation direction. Is it in front or behind?
      const auto x1 = _primitives(0, lo, l);
      const auto x2 = _primitives(0, hi, l);

      // compute intersection location
      const auto xs = (x2 - x1) * (y - y1) / (y2 - y1) + x1;
      if (x > xs)
        return false;
    }
  }

  return true;
}

KOKKOS_FUNCTION inline bool
KokkosSelfShadowSideUserObject::check3DIllumination(const Real3 & qp, const dof_id_type id) const
{
  const auto x = qp(0);
  const auto y = qp(1);
  const auto z = qp(2);

  // Same as MooseUtils fuzzy comparisons, which are not available in device code
  constexpr Real tol = libMesh::TOLERANCE;
  constexpr Real z_tol = libMesh::TOLERANCE * libMesh::TOLERANCE;

  // loop over all triangles until one is found that provides shade
  for (dof_id_type t = 0; t < _primitive_ids.size(); ++t)
  {
    // make sure a side never shades itself
    if (_primitive_ids[t] == id)
      continue;

    const auto x1 = _primitives(0, 0, t);
    const auto x2 = _primitives(0, 1, t);
    const auto x3 = _primitives(0, 2, t);

    const auto y1 = _primitives(1, 0, t);
    const auto y2 = _primitives(1, 1, t);
    const auto y3 = _primitives(1, 2, t);

    // Scale the degeneracy tolerance by the projected-area terms so the check is independent of
    // mesh size.
    const auto denominator_term_1 = (y2 - y3) * (x1 - x3);
    const auto denominator_term_2 = (x3 - x2) * (y1 - y3);
    const auto denominator = denominator_term_1 + denominator_term_2;
    const auto denominator_tolerance =
        tol * (::Kokkos::abs(denominator_term_1) + ::Kokkos::abs(denominator_term_2));
    if (::Kokkos::abs(denominator) <= denominator_tolerance)
      continue;

    // compute barycentric coordinates
    const auto a = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / denominator;
    const auto b = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / denominator;
    const auto c = 1.0 - a - b;

    // Match the robust triangle-intersection predicate by treating barycentric coordinates within
    // libMesh::TOLERANCE of an edge as on the triangle.
    if (a >= -tol && a <= 1 + tol && b >= -tol && b <= 1 + tol && c >= -tol && c <= 1 + tol)
    {
      // Is the intersection it in front or behind the QP? (interpolate z using the barycentric
      // coordinates)
      const auto zs =
          a * _primitives(2, 0, t) + b * _primitives(2, 1, t) + c * _primitives(2, 2, t);
      if (z > zs + z_tol)
        return false;
    }
  }

  return true;
}

template <typename Derived>
KOKKOS_FUNCTION void
KokkosSelfShadowSideUserObject::execute(Datum & datum) const
{
  const auto elem = datum.elemID();
  const auto side = datum.side();
  const dof_id_type id = _side_keys(side, elem);

  // start off assuming no illumination
  unsigned int illumination = 0;

  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
  {
    // the quadrature point in the rotated frame
    const auto p = _rotation * datum.q_point(qp);

    // we set the bit at position qp in the bitmask if the QP is illuminated
    if (_dim == 2 ? check2DIllumination(p, id) : check3DIllumination(p, id))
      illumination |= 1u << qp;
  }

  _illumination(side, elem) = illumination;
}
