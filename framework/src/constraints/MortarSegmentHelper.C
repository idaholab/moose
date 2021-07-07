//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarSegmentHelper.h"
#include <vector>

MortarSegmentHelper::MortarSegmentHelper(const std::vector<Point> secondary_nodes,
                                         const Point & center,
                                         const Point & normal)
  : _center(center), _normal(normal), _debug(false)
{
  _secondary_poly.clear();
  _secondary_poly.reserve(secondary_nodes.size());

  // Get orientation of secondary poly
  Point e1 = secondary_nodes[0] - secondary_nodes[1];
  Point e2 = secondary_nodes[2] - secondary_nodes[1];
  Real orient = e2.cross(e1) * _normal;

  // u and v define the tangent plane of the element (at center)
  // Note we embed orientation into our transformation to make 2D poly always
  // positively oriented
  _u = _normal.cross(secondary_nodes[0] - center).unit();
  _v = (orient > 0) ? _normal.cross(_u).unit() : _u.cross(_normal).unit();

  // Transform problem to 2D plane spanned by u and v
  for (auto node : secondary_nodes)
  {
    Point pt = node - _center;
    _secondary_poly.emplace_back(pt * _u, pt * _v, 0);
  }

  // Initialize area of secondary polygon
  _remaining_area_fraction = 1.0;
  _secondary_area = area(_secondary_poly);
  _scaled_tol = _tolerance * _secondary_area;
}

Point
MortarSegmentHelper::getIntersection(
    const Point & p1, const Point & p2, const Point & q1, const Point & q2, Real & s) const
{
  Point dp = p2 - p1;
  Point dq = q2 - q1;
  Real cp1q1 = p1(0) * q1(1) - p1(1) * q1(0);
  Real cp1q2 = p1(0) * q2(1) - p1(1) * q2(0);
  Real cp1p2 = p1(0) * p2(1) - p1(1) * p2(0);
  Real cq1q2 = q1(0) * q2(1) - q1(1) * q2(0);
  Real alpha = 1. / (dp(0) * dq(1) - dp(1) * dq(0));
  s = -alpha * (cp1q2 - cp1q1 - cq1q2);

  if (alpha > 1e12)
    mooseWarning("MortarSegmentHelper intersection calculation is poorly conditioned");

  return alpha * (cq1q2 * dp - cp1p2 * dq);
}

bool
MortarSegmentHelper::isInsideSecondary(const Point pt) const
{
  for (auto i : make_range(_secondary_poly.size()))
  {
    const Point & q1 = _secondary_poly[(i - 1) % _secondary_poly.size()];
    const Point & q2 = _secondary_poly[i];

    Point e1 = q2 - q1;
    Point e2 = pt - q1;

    // If point corresponds to one of the secondary vertices, skip
    if (e2.norm() < _tolerance)
      return true;

    bool inside = (e1(0) * e2(1) - e1(1) * e2(0)) < _scaled_tol;
    if (!inside)
      return false;
  }
  return true;
}

bool
MortarSegmentHelper::isDisjoint(const std::vector<Point> & poly) const
{
  for (auto i : make_range(poly.size()))
  {
    // Get edge to check
    const Point edg = poly[(i + 1) % poly.size()] - poly[i];
    const Real cp =
        poly[(i + 1) % poly.size()](0) * poly[i](1) - poly[(i + 1) % poly.size()](1) * poly[i](0);

    // If more optimization needed, could store these values for later
    // Check if point is to the left of (or on) clip_edge
    auto is_inside = [&edg, cp](Point & pt, Real tol) {
      return pt(0) * edg(1) - pt(1) * edg(0) + cp < -tol;
    };

    bool all_outside = true;
    for (auto pt : _secondary_poly)
    {
      if (is_inside(pt, _scaled_tol))
        all_outside = false;
    }

    if (all_outside)
      return true;
  }
  return false;
}

void
MortarSegmentHelper::clipPoly(const std::vector<Point> & primary_nodes,
                              std::vector<Point> & clipped_poly) const
{
  // Check orientation of primary_poly
  Point e1 = primary_nodes[0] - primary_nodes[1];
  Point e2 = primary_nodes[2] - primary_nodes[1];

  // Note we use u x v here instead of normal because it may be flipped if secondary elem was
  // negatively oriented
  Real orient = e2.cross(e1) * _u.cross(_v);

  // Get primary_poly (primary is clipping poly). If negatively oriented, reverse
  std::vector<Point> primary_poly;
  const int n_verts = primary_nodes.size();
  for (auto n : make_range(n_verts))
  {
    Point pt = (orient > 0) ? primary_nodes[n] - _center : primary_nodes[n_verts - 1 - n] - _center;
    primary_poly.emplace_back(pt * _u, pt * _v, 0.);
  }

  if (isDisjoint(primary_poly))
    return;

  // Initialize clipped poly with secondary poly (secondary is target poly)
  clipped_poly = _secondary_poly;

  // Loop through clipping edges
  for (auto i : make_range(primary_poly.size()))
  {
    // If clipped poly trivial, return
    if (clipped_poly.size() < 3)
    {
      clipped_poly.clear();
      return;
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
     * since finding intersection is ill-conditioned in this case. Dividing by
     * secondary_area is to non-dimensionalize before tolerancing
     */
    auto is_inside = [&edg, cp](const Point & pt, Real tol) {
      return pt(0) * edg(1) - pt(1) * edg(0) + cp < tol;
    };

    // Loop through edges of target polygon (with previous clippings already included)
    for (auto j : make_range(input_poly.size()))
    {
      // Get target edge
      const Point curr_pt = input_poly[(j + 1) % input_poly.size()];
      const Point prev_pt = input_poly[j];

      // TODO: Don't need to calculate both each loop
      bool is_current_inside = is_inside(curr_pt, _scaled_tol);
      bool is_previous_inside = is_inside(prev_pt, _scaled_tol);

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
}

void
MortarSegmentHelper::plotPoly(const std::vector<Point> & poly) const
{
  if (poly.size() < 2)
    return;

  Moose::out << "plt.plot([";
  for (auto pt : poly)
    Moose::out << pt(0) << ", ";
  Moose::out << poly[0](0) << "], [";
  for (auto pt : poly)
    Moose::out << pt(1) << ", ";
  Moose::out << poly[0](1) << "])" << std::endl;
}

void
MortarSegmentHelper::plotTriangulation(const std::vector<Point> & nodes,
                                       const std::vector<std::array<int, 3>> & elem_to_nodes) const
{
  for (auto el : elem_to_nodes)
  {
    std::vector<Point> poly;
    poly.push_back(nodes[el[0]]);
    poly.push_back(nodes[el[1]]);
    poly.push_back(nodes[el[2]]);
    plotPoly(poly);
  }
}

void
MortarSegmentHelper::triangulatePoly(std::vector<Point> & poly_nodes,
                                     unsigned int offset,
                                     std::vector<std::array<unsigned int, 3>> & tri_map) const
{
  // If fewer than 3 nodes, error
  if (poly_nodes.size() < 3)
  {
    mooseError("Can't triangulate poly with fewer than 3 nodes");
  }
  // If three nodes, already a triangle, simply pass back map
  else if (poly_nodes.size() == 3)
  {
    tri_map.push_back({{0 + offset, 1 + offset, 2 + offset}});
    return;
  }
  // Otherwise add center point node and make simple triangulation
  else
  {
    Point poly_center;
    unsigned int n_verts = poly_nodes.size();

    // Get geometric center of polygon
    for (auto node : poly_nodes)
      poly_center += node;
    poly_center /= n_verts;

    // Add triangles formed by outer edge and center point
    for (auto i : make_range(n_verts))
      tri_map.push_back({{i + offset, (i + 1) % n_verts + offset, n_verts + offset}});

    // Add center point to list of polygon nodes
    poly_nodes.push_back(poly_center);
    return;
  }
}

void
MortarSegmentHelper::getMortarSegments(const std::vector<Point> & primary_nodes,
                                       std::vector<Point> & nodes,
                                       std::vector<std::array<unsigned int, 3>> & elem_to_nodes)
{
  // Clip primary elem against secondary elem
  std::vector<Point> clipped_poly;
  clipPoly(primary_nodes, clipped_poly);
  if (clipped_poly.size() < 3)
    return;

  if (_debug)
  {
    for (auto pt : clipped_poly)
    {
      if (!isInsideSecondary(pt))
        mooseError("Clipped polygon not inside linearized secondary element");
    }
  }

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
  for (auto i : make_range(nodes.size()))
    poly_area += nodes[i](0) * nodes[(i + 1) % nodes.size()](1) -
                 nodes[i](1) * nodes[(i + 1) % nodes.size()](0);
  poly_area *= 0.5;
  return poly_area;
}
