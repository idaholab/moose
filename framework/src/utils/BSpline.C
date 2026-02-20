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
#include "MooseError.h"

using namespace libMesh;

namespace Moose
{
BSpline::BSpline(const unsigned int degree, const std::vector<libMesh::Point> & control_points)
  : _degree(degree), _control_points(control_points), _knot_vector(buildKnotVector())
{
}

/**
 * Evaluate B-Spline at given t value using the control points.
 */
libMesh::Point
BSpline::getPoint(const libMesh::Real t) const
{
  mooseAssert((t >= 0) && (t <= 1), "t should be in [0, 1]. t=" + std::to_string(t));
  libMesh::Point returnPoint(0, 0, 0);
  for (const auto i : index_range(_control_points))
    returnPoint += BSpline::CdBBasis(t, i, _degree) * _control_points[i];

  return returnPoint;
}

/**
 * Creates normalized open uniform knot vector.
 */
std::vector<libMesh::Real>
BSpline::buildKnotVector() const
{
  const unsigned int num_points_left = _control_points.size() - _degree;

  if (num_points_left <= 0)
    mooseError("Number of control points must be greater than or equal to degree + 1!");

  std::vector<libMesh::Real> knot_vector(_degree, 0); // initialize bottom to zeros

  for (const auto i : make_range(num_points_left + 1))
  {
    knot_vector.push_back(i); // count up
    mooseAssert(i < _control_points.size(),
                "i must be less than the number of control points because of 0 indexing.");
  }

  for (const auto i : make_range(_degree))
  {
    libmesh_ignore(i);
    knot_vector.push_back(num_points_left); // append values to create the open knot vector
  }

  // Normalize values s.t. the the max value is 1.
  // This must be done to have a constant domain of t to be between 0 and 1 (inclusive).
  for (libMesh::Real & value : knot_vector)
  {
    value /= num_points_left;
    mooseAssert(value <= 1.0 && value >= 0.0,
                "knot_vector must be normalized to 1 with minimum value 0.");
  }

  return knot_vector;
}

libMesh::Real
BSpline::CdBBasis(const libMesh::Real & t, const unsigned int i, const unsigned int j) const
{
  mooseAssert(i < _knot_vector.size() - 1, "knot index must stay within bounds");
  if (j == 0)
  {
    libMesh::Real basis_return =
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
libMesh::Real
BSpline::firstCoeff(const libMesh::Real & t, const unsigned int i, const unsigned int j) const
{
  mooseAssert(i + j < _knot_vector.size(), "Index must stay within bounds");
  const auto ti = _knot_vector[i];
  const auto tij = _knot_vector[i + j];
  if (ti != tij)
    return (t - ti) / (tij - ti);
  else
    return 0;
}

/**
 * Submethod to aid in computing the basis function in CdBBasis.
 */
libMesh::Real
BSpline::secondCoeff(const libMesh::Real & t, const unsigned int i, const unsigned int j) const
{
  libMesh::Real ti1 = _knot_vector[i + 1];
  libMesh::Real tij1 = _knot_vector[i + j + 1];
  if (ti1 != tij1)
    return (tij1 - t) / (tij1 - ti1);
  else
    return 0;
}
}
