//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Reference for B-Spline: https://mat.fsv.cvut.cz/gcg/sbornik/prochazkova.pdf

#pragma once

#include <vector>
#include "libmesh/point.h"

namespace Moose
{
/**
 * Class implementing a B-Spline curve.
 */
class BSpline
{
public:
  BSpline(const unsigned int degree, const std::vector<libMesh::Point> & control_points);

  /**
   * Evaluate the BSpline interpolation at given value of t.
   *
   * @param t the parameter value, must lie in [0,1]
   * @return libMesh::Point type with format (x,y,z)
   */
  libMesh::Point getPoint(const libMesh::Real t) const;

  /**
   * Get the built knot vector associated with the degree and control points.
   */
  const std::vector<libMesh::Real> & getKnotVector() const { return _knot_vector; }

private:
  /**
   * Internal method for building the knot vector given the degree and control points.
   */
  std::vector<libMesh::Real> buildKnotVector() const;

  /**
   * Evaluates the the basis function for a B-Spline according to the Cox-de-Boor
   * recursive formula.
   *
   * @param t,i,j parameter t in [0,1], index corresponding to which control point, and the degree
   * of the basis funtion
   * @return scalar quantity for the contribution of the basis function at i in the sum.
   */
  libMesh::Real CdBBasis(const libMesh::Real & t, const unsigned int i, const unsigned int j) const;
  /**
   * firstCoeff and secondCoeff are submethods used in CdBBasis routine.
   */
  libMesh::Real
  firstCoeff(const libMesh::Real & t, const unsigned int i, const unsigned int j) const;
  libMesh::Real
  secondCoeff(const libMesh::Real & t, const unsigned int i, const unsigned int j) const;

  /// The polynomial degree.
  const unsigned int _degree;
  /// The control points.
  const std::vector<libMesh::Point> _control_points;
  /// The knot vector.
  const std::vector<libMesh::Real> _knot_vector;
};
}
