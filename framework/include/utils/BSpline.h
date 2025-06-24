//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include "libmesh/point.h"
#include "libMeshReducedNamespace.h"

using namespace libMesh;

namespace Moose
{
class BSpline
{
public:
  BSpline(const unsigned int degree, const std::vector<libMesh::Point> & control_points);

  /**
   * Get the point on the spline evaluated at \p t
   */
  libMesh::Point getPoint(const Real t) const;

  /**
   * Get the built knot vector associated with the degree and control points
   */
  const std::vector<Real> & getKnotVector() const { return _knot_vector; }

private:
  /**
   * Internal method for building the knot vector given the degree and control points.
   */
  std::vector<Real> buildKnotVector() const;

  /**
   * Internal method for evaluating the Cox-de-Boor basis functions.
   * Two additional functions, firstCoeff and secondCoeff, are submethods for CdBBasis
   * function to evaluate front coefficients in the recursive formula.
   */
  Real CdBBasis(const Real & t, const unsigned int i, const unsigned int j) const;
  Real firstCoeff(const Real & t, const unsigned int i, const unsigned int j) const;
  Real secondCoeff(const Real & t, const unsigned int i, const unsigned int j) const;

  /// The polynomial degree.
  const unsigned int _degree;
  /// The control points.
  const std::vector<libMesh::Point> _control_points;
  /// The knot vector.
  const std::vector<Real> _knot_vector;
};
}
