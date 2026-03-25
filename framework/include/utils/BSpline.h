//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libMeshReducedNamespace.h"
#include "libmesh/point.h"
#include <vector>

namespace Moose
{
/**
 * Class implementing a uniform clamped B-Spline curve.
 * Reference used for implementation: https://mat.fsv.cvut.cz/gcg/sbornik/prochazkova.pdf
 * The B-Spline is uniform because of the spacings in the knot vector
 * The B-Spline is clamped because the knot vector starts (and ends) with (degree + 1) 0s (and 1s)
 * Clamped means that it passes through the specified start and end points with specified slopes
 */
class BSpline
{
public:
  BSpline(const unsigned int degree,
          const libMesh::Point & start_point,
          const libMesh::Point & end_point,
          const libMesh::RealVectorValue & start_direction,
          const libMesh::RealVectorValue & end_direction,
          const unsigned int cps_per_half,
          const libMesh::Real sharpness);

  /**
   * Evaluate the BSpline interpolation at given value of t.
   *
   * @param t the parameter value, must lie in [0,1]
   * @return libMesh::Point type with format (x,y,z)
   */
  libMesh::Point getPoint(const libMesh::Real t) const;

private:
  /**
   * Internal method for building the knot vector given the degree and control points.
   */
  std::vector<libMesh::Real> buildKnotVector() const;

  /**
   * Creates a vector control points from the SplineUtils set of functions.
   */
  std::vector<libMesh::Point> createControlPoints() const;

  /**
   * Evaluates the the basis function for a B-Spline according to the Cox-de-Boor
   * recursive formula.
   *
   * @param t parameter t in [0,1]
   * @param i index corresponding to which control point
   * @param j degree of the basis funtion
   * @return scalar quantity for the contribution of the basis function at i in the sum.
   */
  libMesh::Real CdBBasis(const libMesh::Real t, const unsigned int i, const unsigned int j) const;

  /**
   * Submethod used in CdBBasis routine.
   * @param t parameter t in [0,1]
   * @param i index corresponding to which control point
   * @param j degree of the basis funtion
   */
  libMesh::Real firstCoeff(const libMesh::Real t, const unsigned int i, const unsigned int j) const;

  /**
   * Submethod used in CdBBasis routine.
   * @param t parameter t in [0,1]
   * @param i index corresponding to which control point
   * @param j degree of the basis funtion
   */
  libMesh::Real
  secondCoeff(const libMesh::Real t, const unsigned int i, const unsigned int j) const;

  /// The polynomial degree of the B-Spline.
  const unsigned int _degree;
  /// starting point of spline
  const libMesh::Point _start_point;
  /// ending point of spline
  const libMesh::Point _end_point;
  /// starting direction of spline
  const libMesh::RealVectorValue _start_dir;
  /// ending direction of spline
  const libMesh::RealVectorValue _end_dir;
  /// number of control points per half of the spline (vertex is auto-included)
  const unsigned int _cps_per_half;
  /// sharpness of the curve
  const libMesh::Real _sharpness;
  /// The control points.
  const std::vector<libMesh::Point> _control_points;
  /// The knot vector.
  /// Size: degree + n_control_points + 1
  /// The multiplicity of each knot influences the continuity of the spline
  const std::vector<libMesh::Real> _knot_vector;
};
}
