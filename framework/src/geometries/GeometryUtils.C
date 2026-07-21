//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometryUtils.h"
#include "MooseUtils.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace geom_utils
{

bool
isPointZero(const Point & pt)
{
  const Point zero(0.0, 0.0, 0.0);
  return pt.absolute_fuzzy_equals(zero);
}

Point
unitVector(const Point & pt, const std::string & name)
{
  if (isPointZero(pt))
    mooseError("'" + name + "' cannot have zero norm!");

  return pt.unit();
}

Real
minDistanceToPoints(const Point & pt,
                    const std::vector<Point> & candidates,
                    const unsigned int axis)
{
  const auto idx = projectedIndices(axis);

  Real distance = std::numeric_limits<Real>::max();
  for (const auto & c : candidates)
  {
    const Real dx = c(idx.first) - pt(idx.first);
    const Real dy = c(idx.second) - pt(idx.second);
    const Real d = std::sqrt(dx * dx + dy * dy);
    distance = std::min(d, distance);
  }

  return distance;
}

Point
projectPoint(const Real x0, const Real x1, const unsigned int axis)
{
  const auto i = projectedIndices(axis);
  Point point;
  point(i.first) = x0;
  point(i.second) = x1;
  point(axis) = 0.0;

  return point;
}

Real
projectedLineHalfSpace(Point pt1, Point pt2, Point pt3, const unsigned int axis)
{
  // project points onto plane perpendicular to axis
  pt1(axis) = 0.0;
  pt2(axis) = 0.0;
  pt3(axis) = 0.0;

  const auto i = projectedIndices(axis);

  return (pt1(i.first) - pt3(i.first)) * (pt2(i.second) - pt3(i.second)) -
         (pt2(i.first) - pt3(i.first)) * (pt1(i.second) - pt3(i.second));
}

bool
pointInPolygon(const Point & point, const std::vector<Point> & corners, const unsigned int axis)
{
  const auto n_pts = corners.size();

  std::vector<bool> negative_half_space;
  std::vector<bool> positive_half_space;
  for (const auto i : index_range(corners))
  {
    const int next = (i == n_pts - 1) ? 0 : i + 1;
    const auto half = projectedLineHalfSpace(point, corners[i], corners[next], axis);
    negative_half_space.push_back(half < 0);
    positive_half_space.push_back(half > 0);
  }

  const bool negative = std::find(negative_half_space.begin(), negative_half_space.end(), true) !=
                        negative_half_space.end();
  const bool positive = std::find(positive_half_space.begin(), positive_half_space.end(), true) !=
                        positive_half_space.end();

  const bool in_polygon = !(negative && positive);
  if (in_polygon)
    return true;

  if (pointOnEdge(point, corners, axis))
    return true;

  return false;
}

bool
pointOnEdge(const Point & point, const std::vector<Point> & corners, const unsigned int axis)
{
  const auto n_pts = corners.size();
  const auto idx = projectedIndices(axis);

  constexpr Real tol = 1e-8;
  for (const auto i : index_range(corners))
  {
    const int next = (i == n_pts - 1) ? 0 : i + 1;
    const auto & pt1 = corners[i];
    const auto & pt2 = corners[next];
    const bool close_to_line = projectedDistanceFromLine(point, pt1, pt2, axis) < tol;

    // we can stop early if we know we're not close to the line
    if (!close_to_line)
      continue;

    // check that the point is "between" the two points; TODO: first pass
    // we can just compare x and y coordinates
    const bool between_points = (point(idx.first) >= std::min(pt1(idx.first), pt2(idx.first))) &&
                                (point(idx.first) <= std::max(pt1(idx.first), pt2(idx.first))) &&
                                (point(idx.second) >= std::min(pt1(idx.second), pt2(idx.second))) &&
                                (point(idx.second) <= std::max(pt1(idx.second), pt2(idx.second)));

    // point needs to be close to the line AND "between" the two points
    if (close_to_line && between_points)
      return true;
  }

  return false;
}

std::pair<unsigned int, unsigned int>
projectedIndices(const unsigned int axis)
{
  std::pair<unsigned int, unsigned int> indices;

  if (axis == 0)
  {
    indices.first = 1;
    indices.second = 2;
  }
  else if (axis == 1)
  {
    indices.first = 0;
    indices.second = 2;
  }
  else
  {
    indices.first = 0;
    indices.second = 1;
  }

  return indices;
}

Point
projectedUnitNormal(Point pt1, Point pt2, const unsigned int axis)
{
  // project the points to the plane perpendicular to the axis
  pt1(axis) = 0.0;
  pt2(axis) = 0.0;

  const auto i = projectedIndices(axis);

  const Real dx = pt2(i.first) - pt1(i.first);
  const Real dy = pt2(i.second) - pt1(i.second);

  const Point normal = projectPoint(dy, -dx, axis);
  const Point gap_line = pt2 - pt1;

  const auto cross_product = gap_line.cross(normal);

  if (cross_product(axis) > 0)
    return normal.unit();
  else
    return projectPoint(-dy, dx, axis).unit();
}

Real
distanceFromLine(const Point & pt, const Point & line0, const Point & line1)
{
  const Point a = pt - line0;
  const Point b = pt - line1;
  const Point c = line1 - line0;

  return (a.cross(b).norm()) / c.norm();
}

Real
projectedDistanceFromLine(Point pt, Point line0, Point line1, const unsigned int axis)
{
  // project all the points to the plane perpendicular to the axis
  pt(axis) = 0.0;
  line0(axis) = 0.0;
  line1(axis) = 0.0;

  return distanceFromLine(pt, line0, line1);
}

std::vector<Point>
polygonCorners(const unsigned int num_sides, const Real radius, const unsigned int axis)
{
  std::vector<Point> corners;
  const Real theta = 2.0 * M_PI / num_sides;
  const Real first_angle = M_PI / 2.0 - theta / 2.0;

  for (const auto i : make_range(num_sides))
  {
    const Real angle = first_angle + i * theta;
    const Real x = radius * cos(angle);
    const Real y = radius * sin(angle);

    corners.push_back(projectPoint(x, y, axis));
  }

  return corners;
}

Point
rotatePointAboutAxis(const Point & p, const Real angle, const Point & axis)
{
  const Real cos_theta = cos(angle);
  const Real sin_theta = sin(angle);

  Point pt;
  const Real xy = axis(0) * axis(1);
  const Real xz = axis(0) * axis(2);
  const Real yz = axis(1) * axis(2);

  const Point x_op(cos_theta + axis(0) * axis(0) * (1.0 - cos_theta),
                   xy * (1.0 - cos_theta) - axis(2) * sin_theta,
                   xz * (1.0 - cos_theta) + axis(1) * sin_theta);

  const Point y_op(xy * (1.0 - cos_theta) + axis(2) * sin_theta,
                   cos_theta + axis(1) * axis(1) * (1.0 - cos_theta),
                   yz * (1.0 - cos_theta) - axis(0) * sin_theta);

  const Point z_op(xz * (1.0 - cos_theta) - axis(1) * sin_theta,
                   yz * (1.0 - cos_theta) + axis(0) * sin_theta,
                   cos_theta + axis(2) * axis(2) * (1.0 - cos_theta));

  pt(0) = x_op * p;
  pt(1) = y_op * p;
  pt(2) = z_op * p;
  return pt;
}

std::vector<Point>
boxCorners(const libMesh::BoundingBox & box, const Real factor)
{
  Point diff = (box.max() - box.min()) / 2.0;
  const Point origin = box.min() + diff;

  // Rescale side length of box by specified factor
  diff *= factor;

  // Vectors for sides of box
  const Point dx(2.0 * diff(0), 0.0, 0.0);
  const Point dy(0.0, 2.0 * diff(1), 0.0);
  const Point dz(0.0, 0.0, 2.0 * diff(2));

  std::vector<Point> verts(8, origin - diff);
  const unsigned int pts_per_dim = 2;
  for (const auto z : make_range(pts_per_dim))
    for (const auto y : make_range(pts_per_dim))
      for (const auto x : make_range(pts_per_dim))
        verts[pts_per_dim * pts_per_dim * z + pts_per_dim * y + x] += x * dx + y * dy + z * dz;

  return verts;
}

bool
arePointsColinear(const Point & p1, const Point & p2, const Point & p3)
{
  const Point v1 = p2 - p1;
  const Point v2 = p3 - p1;
  const Point cross_prod = v1.cross(v2);

  return MooseUtils::absoluteFuzzyEqual(cross_prod.norm(), 0.0);
}

bool
segmentsIntersect(const Point & p1, const Point & p2, const Point & p3, const Point & p4)
{
  mooseAssert(
      MooseUtils::absoluteFuzzyEqual(p1(2), 0.0) && MooseUtils::absoluteFuzzyEqual(p2(2), 0.0) &&
          MooseUtils::absoluteFuzzyEqual(p3(2), 0.0) && MooseUtils::absoluteFuzzyEqual(p4(2), 0.0),
      "segmentsIntersect only works in 2D (x-y plane)");

  mooseAssert(MooseUtils::absoluteFuzzyGreaterThan((p1 - p2).norm(), 0.0) &&
                  MooseUtils::absoluteFuzzyGreaterThan((p3 - p4).norm(), 0.0),
              "Zero length segments are not allowed in segmentsIntersect");

  const Real a1 = p2(1) - p1(1);
  const Real b1 = p1(0) - p2(0);
  const Real c1 = p2(0) * p1(1) - p1(0) * p2(1);

  const Real a2 = p4(1) - p3(1);
  const Real b2 = p3(0) - p4(0);
  const Real c2 = p4(0) * p3(1) - p3(0) * p4(1);

  const Real denom = a1 * b2 - a2 * b1;
  Point intersection_pt;
  // Parallel case
  if (MooseUtils::absoluteFuzzyEqual(denom, 0.0))
  {
    if (arePointsColinear(p1, p2, p3))
    {
      // for colinear segments, we construct a "virtual" intersection point using weighted average
      // In that case, the virtual point will always lie outside of both segments unless they
      // overlap
      const Point p12 = (p1 + p2) / 2.0;
      const Point p34 = (p3 + p4) / 2.0;
      const Real dist12 = (p1 - p2).norm();
      const Real dist34 = (p3 - p4).norm();
      intersection_pt = p12 + (p34 - p12) * (dist12 / (dist12 + dist34));
    }
    else
      return false;
  }
  else
  {
    intersection_pt = Point((b1 * c2 - b2 * c1) / denom, (a2 * c1 - a1 * c2) / denom, 0.0);
  }

  const Real ratio_p1p2 = (intersection_pt - p1) * (p2 - p1) / ((p2 - p1).norm_sq());
  const Real ratio_p3p4 = (intersection_pt - p3) * (p4 - p3) / ((p4 - p3).norm_sq());

  if (MooseUtils::absoluteFuzzyGreaterEqual(ratio_p1p2, 0.0) &&
      MooseUtils::absoluteFuzzyLessEqual(ratio_p1p2, 1.0) &&
      MooseUtils::absoluteFuzzyGreaterEqual(ratio_p3p4, 0.0) &&
      MooseUtils::absoluteFuzzyLessEqual(ratio_p3p4, 1.0))
    return true;
  else
    return false;
}

Real
pointSegmentDistanceSq(const Point & point, const Point & a, const Point & b)
{
  const Point ab = b - a;
  const auto length_sq = ab.norm_sq();
  if (length_sq <= std::numeric_limits<Real>::epsilon())
    return (point - a).norm_sq();

  const auto t = std::clamp(((point - a) * ab) / length_sq, 0.0, 1.0);
  const Point projection = a + t * ab;
  return (point - projection).norm_sq();
}

Real
pointTriangleDistanceSq(const Point & point, const Point & v0, const Point & v1, const Point & v2)
{
  const Point ab = v1 - v0;
  const Point ac = v2 - v0;
  const Point ap = point - v0;
  const Real d1 = ab * ap;
  const Real d2 = ac * ap;
  if (d1 <= 0.0 && d2 <= 0.0)
    return (point - v0).norm_sq();

  const Point bp = point - v1;
  const Real d3 = ab * bp;
  const Real d4 = ac * bp;
  if (d3 >= 0.0 && d4 <= d3)
    return (point - v1).norm_sq();

  const Real vc = d1 * d4 - d3 * d2;
  if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0)
  {
    const Real v = d1 / (d1 - d3);
    const Point projection = v0 + v * ab;
    return (point - projection).norm_sq();
  }

  const Point cp = point - v2;
  const Real d5 = ab * cp;
  const Real d6 = ac * cp;
  if (d6 >= 0.0 && d5 <= d6)
    return (point - v2).norm_sq();

  const Real vb = d5 * d2 - d1 * d6;
  if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0)
  {
    const Real w = d2 / (d2 - d6);
    const Point projection = v0 + w * ac;
    return (point - projection).norm_sq();
  }

  const Real va = d3 * d6 - d5 * d4;
  if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0)
  {
    const Point bc = v2 - v1;
    const Real w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
    const Point projection = v1 + w * bc;
    return (point - projection).norm_sq();
  }

  const Real denom = 1.0 / (va + vb + vc);
  const Real v = vb * denom;
  const Real w = vc * denom;
  const Point projection = v0 + ab * v + ac * w;
  return (point - projection).norm_sq();
}

Real
solidAngle(const Point & point, const Point & v0, const Point & v1, const Point & v2)
{
  const Point a = v0 - point;
  const Point b = v1 - point;
  const Point c = v2 - point;

  const Real la = a.norm();
  const Real lb = b.norm();
  const Real lc = c.norm();

  const Real numerator = a * (b.cross(c));
  const Real denominator = la * lb * lc + (a * b) * lc + (b * c) * la + (c * a) * lb;

  return 2.0 * std::atan2(numerator, denominator);
}
} // end namespace geom_utils
