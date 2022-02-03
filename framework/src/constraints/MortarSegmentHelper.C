//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "MortarSegmentHelper.h"
#include "MooseError.h"

#include "libmesh/int_range.h"

using namespace libMesh;

MortarSegmentHelper::MortarSegmentHelper(const std::vector<Point> secondary_nodes,
                                         const Point & center,
                                         const Point & normal)
  : _center(center), _normal(normal), _debug(false)
{
  _secondary_poly.clear();
  _secondary_poly.reserve(secondary_nodes.size());

  // Get orientation of secondary poly
  const Point e1 = secondary_nodes[0] - secondary_nodes[1];
  const Point e2 = secondary_nodes[2] - secondary_nodes[1];
  const Real orient = e2.cross(e1) * _normal;

  // u and v define the tangent plane of the element (at center)
  // Note we embed orientation into our transformation to make 2D poly always
  // positively oriented
  _u = _normal.cross(secondary_nodes[0] - center).unit();
  _v = (orient > 0) ? _normal.cross(_u).unit() : _u.cross(_normal).unit();

  // Transform problem to 2D plane spanned by u and v
  for (const auto & node : secondary_nodes)
  {
    Point pt = node - _center;
    _secondary_poly.emplace_back(pt * _u, pt * _v, 0);
  }

  // Initialize area of secondary polygon
  _remaining_area_fraction = 1.0;
  _secondary_area = area(_secondary_poly);

  // Tolerance for quantities with area dimensions
  _area_tol = _tolerance * _secondary_area;

  // Tolerance for quantites with length dimensions
  _length_tol = _tolerance * std::sqrt(_secondary_area);
}

Point
MortarSegmentHelper::getIntersection(
    const Point & p1, const Point & p2, const Point & q1, const Point & q2, Real & s) const
{
  const Point dp = p2 - p1;
  const Point dq = q2 - q1;
  const Real cp1q1 = p1(0) * q1(1) - p1(1) * q1(0);
  const Real cp1q2 = p1(0) * q2(1) - p1(1) * q2(0);
  const Real cq1q2 = q1(0) * q2(1) - q1(1) * q2(0);
  const Real alpha = 1. / (dp(0) * dq(1) - dp(1) * dq(0));
  s = -alpha * (cp1q2 - cp1q1 - cq1q2);

  // Intersection should be between p1 and p2, if it's not (due to poor conditioning), simply
  // move it to one of the end points
  s = s > 1 ? 1. : s;
  s = s < 0 ? 0. : s;
  return p1 + s * dp;
}

bool
MortarSegmentHelper::isInsideSecondary(const Point & pt) const
{
  for (auto i : index_range(_secondary_poly))
  {
    const Point & q1 = _secondary_poly[i];
    const Point & q2 = _secondary_poly[(i + 1) % _secondary_poly.size()];

    const Point e1 = q2 - q1;
    const Point e2 = pt - q1;

    // If point corresponds to one of the secondary vertices, skip
    if (e2.norm() < _tolerance)
      return true;

    bool inside = (e1(0) * e2(1) - e1(1) * e2(0)) < _area_tol;
    if (!inside)
      return false;
  }
  return true;
}

bool
MortarSegmentHelper::isDisjoint(const std::vector<Point> & poly) const
{
  for (auto i : index_range(_secondary_poly))
  {
    // Get edge to check
    const Point & q1 = _secondary_poly[i];
    const Point & q2 = _secondary_poly[(i + 1) % _secondary_poly.size()];
    const Point edg = q2 - q1;
    const Real cp = q2(0) * q1(1) - q2(1) * q1(0);

    // If more optimization needed, could store these values for later
    // Check if point is to the left of (or on) clip_edge
    auto is_inside = [&edg, cp](Point & pt, Real tol)
    { return pt(0) * edg(1) - pt(1) * edg(0) + cp < -tol; };

    bool all_outside = true;
    for (auto pt : poly)
      if (is_inside(pt, _area_tol))
        all_outside = false;

    if (all_outside)
      return true;
  }
  return false;
}

std::vector<Point>
MortarSegmentHelper::clipPoly(const std::vector<Point> & primary_nodes) const
{
  // Check orientation of primary_poly
  const Point e1 = primary_nodes[0] - primary_nodes[1];
  const Point e2 = primary_nodes[2] - primary_nodes[1];

  // Note we use u x v here instead of normal because it may be flipped if secondary elem was
  // negatively oriented
  const Real orient = e2.cross(e1) * _u.cross(_v);

  // Get primary_poly (primary is clipping poly). If negatively oriented, reverse
  std::vector<Point> primary_poly;
  const int n_verts = primary_nodes.size();
  for (auto n : index_range(primary_nodes))
  {
    Point pt = (orient > 0) ? primary_nodes[n] - _center : primary_nodes[n_verts - 1 - n] - _center;
    primary_poly.emplace_back(pt * _u, pt * _v, 0.);
  }

  if (isDisjoint(primary_poly))
  {
    primary_poly.clear();
    return primary_poly;
  }

  // Initialize clipped poly with secondary poly (secondary is target poly)
  std::vector<Point> clipped_poly = _secondary_poly;

  // Loop through clipping edges
  for (auto i : index_range(primary_poly))
  {
    // If clipped poly trivial, return
    if (clipped_poly.size() < 3)
    {
      clipped_poly.clear();
      return clipped_poly;
    }

    // Set input poly to current clipped poly
    std::vector<Point> input_poly(clipped_poly);
    clipped_poly.clear();

    // Get clipping edge
    const Point & clip_pt1 = primary_poly[i];
    const Point & clip_pt2 = primary_poly[(i + 1) % primary_poly.size()];
    const Point edg = clip_pt2 - clip_pt1;
    const Real cp = clip_pt2(0) * clip_pt1(1) - clip_pt2(1) * clip_pt1(0);

    // Check if point is to the left of (or on) clip_edge
    /*
     * Note that use of tolerance here is to avoid degenerate case when lines are
     * essentially on top of each other (common when meshes match across interface)
     * since finding intersection is ill-conditioned in this case.
     */
    auto is_inside = [&edg, cp](const Point & pt, Real tol)
    { return pt(0) * edg(1) - pt(1) * edg(0) + cp < tol; };

    // Loop through edges of target polygon (with previous clippings already included)
    for (auto j : index_range(input_poly))
    {
      // Get target edge
      const Point curr_pt = input_poly[(j + 1) % input_poly.size()];
      const Point prev_pt = input_poly[j];

      // TODO: Don't need to calculate both each loop
      const bool is_current_inside = is_inside(curr_pt, _area_tol);
      const bool is_previous_inside = is_inside(prev_pt, _area_tol);

      if (is_current_inside)
      {
        if (!is_previous_inside)
        {
          Real s;
          Point intersect = getIntersection(prev_pt, curr_pt, clip_pt1, clip_pt2, s);

          /*
           * s is the fraction of distance along clip poly edge that intersection lies
           * It is used here to avoid degenerate polygon cases. For example, consider a
           * case like:
           *          o
           *          |    (inside)
           *    ------|------
           *          |    (outside)
           * when the distance is small (< 1e-7) we don't want to to add both the point
           * and intersection. Also note that when distance on the scale of 1e-7,
           * area on scale of 1e-14 so is insignificant if this results in dropping
           * a tri (for example if next edge crosses again)
           */
          if (s < (1 - _tolerance))
            clipped_poly.push_back(intersect);
        }
        clipped_poly.push_back(curr_pt);
      }
      else if (is_previous_inside)
      {
        Real s;
        Point intersect = getIntersection(prev_pt, curr_pt, clip_pt1, clip_pt2, s);
        if (s > _tolerance)
          clipped_poly.push_back(intersect);
      }
    }
  }

  // return clipped_poly;
  // Make sure final clipped poly is not trivial
  if (clipped_poly.size() < 3)
  {
    clipped_poly.clear();
    return clipped_poly;
  }

  // Clean up result by removing any duplicate nodes
  std::vector<Point> cleaned_poly;

  cleaned_poly.push_back(clipped_poly.back());
  for (auto i : make_range(clipped_poly.size() - 1))
  {
    const Point prev_pt = cleaned_poly.back();
    const Point curr_pt = clipped_poly[i];

    // If points are sufficiently distanced, add to output
    if ((curr_pt - prev_pt).norm() > _length_tol)
      cleaned_poly.push_back(curr_pt);
  }

  return cleaned_poly;
}

void
MortarSegmentHelper::triangulatePoly(std::vector<Point> & poly_nodes,
                                     const unsigned int offset,
                                     std::vector<std::vector<unsigned int>> & tri_map) const
{
  // Note offset is important because it converts from list of nodes on local poly to list of
  // nodes on entire element. This is a sloppy and error-prone way to do this.
  // Could be redesigned by just building and adding elements inside helper, but leaving for now
  // to keep buildMortarSegmentMesh similar for 2D and 3D

  // Fewer than 3 nodes can't be triangulated
  if (poly_nodes.size() < 3)
    mooseError("Can't triangulate poly with fewer than 3 nodes");
  // If 3 nodes, already a triangle, simply pass back map
  else if (poly_nodes.size() == 3)
  {
    tri_map.push_back({0 + offset, 1 + offset, 2 + offset});
    return;
  }
  // Otherwise use simple center point triangulation
  // Note: Could use better algorithm to reduce number of triangles
  //    - Delaunay produces good quality triangulation but might be a bit overkill
  //    - Monotone polygon triangulation algorithm is O(N) but quality is not guaranteed
  else
  {
    Point poly_center;
    const unsigned int n_verts = poly_nodes.size();

    // Get geometric center of polygon
    for (auto node : poly_nodes)
      poly_center += node;
    poly_center /= n_verts;

    // Add triangles formed by outer edge and center point
    for (auto i : make_range(n_verts))
      tri_map.push_back({i + offset, (i + 1) % n_verts + offset, n_verts + offset});

    // Add center point to list of polygon nodes
    poly_nodes.push_back(poly_center);
    return;
  }
}

void
MortarSegmentHelper::getMortarSegments(const std::vector<Point> & primary_nodes,
                                       std::vector<Point> & nodes,
                                       std::vector<std::vector<unsigned int>> & elem_to_nodes)
{
  // Clip primary elem against secondary elem
  std::vector<Point> clipped_poly = clipPoly(primary_nodes);
  if (clipped_poly.size() < 3)
    return;

  if (_debug)
    for (auto pt : clipped_poly)
      if (!isInsideSecondary(pt))
        mooseError("Clipped polygon not inside linearized secondary element");

  // Compute area of clipped polygon, update remaining area fraction
  _remaining_area_fraction -= area(clipped_poly) / _secondary_area;

  // Triangulate clip polygon
  triangulatePoly(clipped_poly, nodes.size(), elem_to_nodes);

  // Transform clipped poly back to (linearized) 3d and append to list
  for (auto pt : clipped_poly)
    nodes.emplace_back((pt(0) * _u) + (pt(1) * _v) + _center);
}

Real
MortarSegmentHelper::area(const std::vector<Point> & nodes) const
{
  Real poly_area = 0;
  for (auto i : index_range(nodes))
    poly_area += nodes[i](0) * nodes[(i + 1) % nodes.size()](1) -
                 nodes[i](1) * nodes[(i + 1) % nodes.size()](0);
  poly_area *= 0.5;
  return poly_area;
}
