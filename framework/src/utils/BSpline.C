//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BSpline.h"
#include "libMeshReducedNamespace.h"
#include "MooseError.h"
#include "SplineUtils.h"

using namespace libMesh;

namespace Moose
{
BSpline::BSpline(const unsigned int degree,
                 const libMesh::Point & start_point,
                 const libMesh::Point & end_point,
                 const libMesh::RealVectorValue & start_direction,
                 const libMesh::RealVectorValue & end_direction,
                 const unsigned int cps_per_half,
                 const libMesh::Real sharpness)
  : _degree(degree),
    _start_point(start_point),
    _end_point(end_point),
    _start_dir(start_direction),
    _end_dir(end_direction),
    _cps_per_half(cps_per_half),
    _sharpness(sharpness),
    _control_points(createControlPoints()),
    _knot_vector(buildKnotVector())
{
}

libMesh::Point
BSpline::getPoint(const libMesh::Real t) const
{
  mooseAssert((t >= 0) && (t <= 1), "t should be in [0, 1]. t=" + std::to_string(t));
  libMesh::Point returnPoint(0, 0, 0);
  if (t == 1.0)
    return _control_points.back();
  for (const auto i : index_range(_control_points))
    returnPoint += BSpline::CdBBasis(t, i, _degree) * _control_points[i];

  return returnPoint;
}

std::vector<libMesh::Point>
BSpline::createControlPoints() const
{
  std::vector<libMesh::Point> cps;
  /// call SplineUtils function
  cps = SplineUtils::bSplineControlPoints(
      _start_point, _end_point, _start_dir, _end_dir, _cps_per_half, _sharpness);
  return cps;
}

std::vector<libMesh::Real>
BSpline::buildKnotVector() const
{
  std::vector<libMesh::Real> knot_vector(_degree, 0); // initialize bottom to zeros

  const unsigned int num_points_left = _control_points.size() + 1 - _degree;
  if (_control_points.size() < _degree + 1)
    mooseError("Number of control points must be greater than or equal to degree + 1!");

  for (const auto i : make_range(num_points_left))
  {
    knot_vector.push_back(i); // count up
    mooseAssert(i < _control_points.size(),
                "i must be less than the number of control points because of 0 indexing.");
  }

  // Filling with int(degree + 1) 0s at the beginning and 1s at the end makes the B-spline go
  // through the start and end point Note: the "+1" comes from the loop above which starts at 0 and
  // ends at num_points_left - 1
  for (const auto i : make_range(_degree))
  {
    libmesh_ignore(i);
    knot_vector.push_back(num_points_left - 1);
  }

  // Normalize values s.t. the the max value is 1.
  // This must be done to have a constant domain of t to be between 0 and 1 (inclusive).
  // NOTE: the spacing is uniform, making this a uniform BSpline
  for (auto & value : knot_vector)
  {
    value /= (num_points_left - 1);
    mooseAssert(value <= 1.0 && value >= 0.0,
                "knot_vector must be normalized to 1 with minimum value 0.");
  }

  return knot_vector;
}

libMesh::Real
BSpline::CdBBasis(const libMesh::Real t, const unsigned int i, const unsigned int j) const
{
  mooseAssert(i + 1 < _knot_vector.size(), "Out of bounds access in knot vector");
  mooseAssert(_knot_vector[i] <= _knot_vector[i + 1],
              "Knot vector is ill-formatted. Ensure values are increasing." +
                  Moose::stringify(_knot_vector));
  if (j == 0)
    return ((t < _knot_vector[i + 1]) && (_knot_vector[i] <= t)) ? 1 : 0;
  else
    return BSpline::firstCoeff(t, i, j) * BSpline::CdBBasis(t, i, j - 1) +
           BSpline::secondCoeff(t, i, j) * BSpline::CdBBasis(t, i + 1, j - 1);
}

libMesh::Real
BSpline::firstCoeff(const libMesh::Real t, const unsigned int i, const unsigned int j) const
{
  mooseAssert(i + j < _knot_vector.size(), "Out of bounds access in knot vector");
  const auto ti = _knot_vector[i];
  const auto tij = _knot_vector[i + j];
  if (ti != tij)
    return (t - ti) / (tij - ti);
  else
    return 0;
}

libMesh::Real
BSpline::secondCoeff(const libMesh::Real t, const unsigned int i, const unsigned int j) const
{
  mooseAssert(i + j + 1 < _knot_vector.size(), "Out of bounds access in knot vector");
  libMesh::Real ti1 = _knot_vector[i + 1];
  libMesh::Real tij1 = _knot_vector[i + j + 1];
  if (ti1 != tij1)
    return (tij1 - t) / (tij1 - ti1);
  else
    return 0;
}
}
