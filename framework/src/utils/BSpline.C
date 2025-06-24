//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BSpline.h"
#include "libmesh/point.h"
#include "libMeshReducedNamespace.h"

using namespace libMesh;

namespace Moose
{
BSpline::BSpline(const unsigned int degree, const std::vector<libMesh::Point> & control_points)
  : _degree(degree), _control_points(control_points), _knot_vector(buildKnotVector())

{
}

/**
 * Evaluate the BSpline interpolation at given value of t.
 *
 * @param t the parameter value, must lie in [0,1]
 * @return libMesh::Point type with format (x,y,z)
 */
libMesh::Point
BSpline::getPoint(const Real t) const
{
  libMesh::Point returnPoint(0, 0, 0);
  for (const auto i : index_range(_control_points))
    returnPoint += BSpline::CdBBasis(t, i, _degree) * _control_points[i];
  return returnPoint;
}

/**
 * Creates normalized knot vector.
 */
std::vector<Real>
BSpline::buildKnotVector() const
{
  const unsigned int num_points_left = _control_points.size() - _degree;

  if (num_points_left <= 0)
    mooseError("Number of control points must be equal to degree + 1!");

  std::vector<Real> knot_vector(_degree, 0); // initialize bottom to zeros
  for (const auto i : make_range(num_points_left))
    knot_vector.push_back(i); // count up

  for (const auto i : make_range(_degree))
  {
    libmesh_ignore(i);
    knot_vector.push_back(num_points_left); // append values to create the open knot vector
  }

  // Normalize values s.t. the the max value is 1.
  // This must be done to have a constant domain of t to be between 0 and 1 (inclusive).
  for (Real & value : knot_vector)
    value /= num_points_left;
  return knot_vector;
}

/**
 * Evaluates the the basis function for a B-Spline according to the Cox-de-Boor
 * recursive formula.
 *
 * @param t,i,j parameter t in [0,1], index corresponding to which control point, and the degree of
 * the basis funtion
 * @return scalar quantity for the contribution of the basis function at i in the sum.
 */
Real
BSpline::CdBBasis(const Real & t, const unsigned int i, const unsigned int j) const
{
  if (j == 0)
  {
    Real basis_return =
        (t <= _knot_vector[i + 1] && _knot_vector[i] <= t && _knot_vector[i] < _knot_vector[i + 1])
            ? 1
            : 0;
    return basis_return;
  }
  else
    return BSpline::firstCoeff(t, i, j) * BSpline::CdBBasis(t, i, j - 1) +
           BSpline::secondCoeff(t, i, j) * BSpline::CdBBasis(t, i + 1, j - 1);
}

/**
 * Submethod to aid in computing the basis function in CdBBasis.
 */
Real
BSpline::firstCoeff(const Real & t, const unsigned int i, const unsigned int j) const
{
  Real ti = _knot_vector[i];
  Real tij = _knot_vector[i + j];
  if (ti != tij)
    return (t - ti) / (tij - ti);
  else
    return 0;
}

/**
 * Submethod to aid in computing the basis function in CdBBasis.
 */
Real
BSpline::secondCoeff(const Real & t, const unsigned int i, const unsigned int j) const
{
  Real ti1 = _knot_vector[i + 1];
  Real tij1 = _knot_vector[i + j + 1];
  if (ti1 != tij1)
    return (tij1 - t) / (tij1 - ti1);
  else
    return 0;
}
}
