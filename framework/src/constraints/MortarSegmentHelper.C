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

MortarSegmentHelper::MortarSegmentHelper(const Elem * secondary_elem_ptr, Point & center, Point & normal) :
  _secondary_elem_ptr(secondary_elem_ptr),
  _center(center),
  _normal(normal),
  _debug(false)
{
  _secondary_poly.clear();
  _secondary_poly.reserve(secondary_elem_ptr->n_vertices());

  // Get orientation of secondary poly
  Point e1 = _secondary_elem_ptr->point(0) - _secondary_elem_ptr->point(1);
  Point e2 = _secondary_elem_ptr->point(2) - _secondary_elem_ptr->point(1);
  Real orient = e2.cross(e1) * _normal;

  // u and v define the tangent plane of the element (at center)
  // Note we embed orientation into our transformation to make 2D poly always
  // positively oriented
  _u = _normal.cross(_secondary_elem_ptr->point(0) - center).unit();
  _v = (orient > 0) ? _normal.cross(_u).unit() : _u.cross(_normal).unit();

  // Transform problem to 2D plane spanned by u and v
  for (auto n : make_range(_secondary_elem_ptr->n_vertices()))
  {
    Point pt = _secondary_elem_ptr->point(n) - _center;
    _secondary_poly.emplace_back(pt * _u, pt * _v, 0);
  }
}


Point
MortarSegmentHelper::getIntersection(Point & p1, Point & p2, Point & q1, Point & q2, Real & s)
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
MortarSegmentHelper::isInsideSecondary(Point & pt)
{
  for (auto i : make_range(_secondary_poly.size()))
  {
    Point & q1 = _secondary_poly[(i - 1) % _secondary_poly.size()];
    Point & q2 = _secondary_poly[i];

    Point e1 = q2 - q1;
    Point e2 = pt - q1;

    // If point corresponds to one of the secondary vertices, skip
    if (e2.norm() < 1e-8)
      return true;

    bool inside = (e1(0) * e2(1) - e1(1) * e2(0)) < 1e-8;
    if (!inside)
      return false;
  }
  return true;
}


bool
MortarSegmentHelper::isDisjoint(std::vector<Point> & poly)
{
  for (auto i : make_range(poly.size()))
  {
    // Get edge to check
    const Point edg = poly[(i + 1) % poly.size()] - poly[i];
    const Real cp = poly[(i + 1) % poly.size()](0) * poly[i](1)
                  - poly[(i + 1) % poly.size()](1) * poly[i](0);

    // If more optimization needed, could store these values for later
    // Check if point is to the left of (or on) clip_edge
    auto is_inside = [&edg, cp](Point & pt) {
      return pt(0) * edg(1) - pt(1) * edg(0) + cp < -1e-8;
    };

    bool all_outside = true;
    for (auto pt : _secondary_poly)
    {
      if(is_inside(pt))
        all_outside = false;
    }

    if (all_outside)
      return true;
  }
  return false;
}


void
MortarSegmentHelper::clipPoly(const Elem * primary_elem_ptr, std::vector<Point> & clipped_poly)
{
  // Check orientation of primary_poly
  Point e1 = primary_elem_ptr->point(0) - primary_elem_ptr->point(1);
  Point e2 = primary_elem_ptr->point(2) - primary_elem_ptr->point(1);
  Real orient = e2.cross(e1) * _u.cross(_v);

  // Get primary_poly (primary is clipping poly). If negatively oriented, reverse
  std::vector<Point> primary_poly;
  const int n_verts = primary_elem_ptr->n_vertices();
  for (auto n : make_range(n_verts))
  {
    Point pt = (orient > 0) ? primary_elem_ptr->point(n) - _center :
                              primary_elem_ptr->point(n_verts - 1 - n) - _center;
    primary_poly.emplace_back(pt * _u, pt * _v, 0.);
  }

  if (isDisjoint(primary_poly))
    return;

  // Initialize clipped poly with secondary poly (secondary is target poly)
  clipped_poly = _secondary_poly;

  // Loop through clipping edges
  for (auto i : make_range(primary_poly.size()))
  {
    if (clipped_poly.size() < 3)
    {
      clipped_poly.clear();
    }

    // Set input poly to current clipped poly
    std::vector<Point> input_poly(clipped_poly);
    clipped_poly.clear();

    // If clipped poly trivial, return
    if (input_poly.size() < 3)
      return;

    // Get clipping edge
    Point & clip_pt1 = primary_poly[i];
    Point & clip_pt2 = primary_poly[(i + 1) % primary_poly.size()];
    const Point edg = clip_pt2 - clip_pt1;
    const Real cp = clip_pt2(0) * clip_pt1(1) - clip_pt2(1) * clip_pt1(0);

    // Check if point is to the left of (or on) clip_edge
    auto is_inside = [&edg, cp](Point & pt) {
      return pt(0) * edg(1) - pt(1) * edg(0) + cp < 1e-8;
    };

    // Loop through edges of target polygon (with previous clippings already included)
    for (auto j : make_range(input_poly.size()))
    {
      // Get target edge
      Point & curr_pt = input_poly[(j + 1) % input_poly.size()];
      Point & prev_pt = input_poly[j];
      // std::cout << "current: " << curr_pt << std::endl;

      // TODO: Don't need to calculate both each time
      bool is_current_inside = is_inside(curr_pt);
      bool is_previous_inside = is_inside(prev_pt);

      if (is_current_inside)
      {
        if (!is_previous_inside)
        {
          Real s;
          Point intersect = getIntersection(prev_pt, curr_pt, clip_pt1, clip_pt2, s);
          if (s < (1 - 1e-6))
            clipped_poly.push_back(intersect);
        }
        clipped_poly.push_back(curr_pt);
      }
      else
        if (is_previous_inside)
        {
          Real s;
          Point intersect = getIntersection(prev_pt, curr_pt, clip_pt1, clip_pt2, s);
          if (s > 1e-6)
            clipped_poly.push_back(intersect);
        }
    }
  }
}


void
MortarSegmentHelper::plotPoly(std::vector<Point> & poly)
{
  if (poly.size() < 2)
    return;

  std::cout << "plt.plot([";
  for (auto pt : poly)
    std::cout << pt(0) << ", ";
  std::cout << poly[0](0) << "], [";
  for (auto pt : poly)
    std::cout << pt(1) << ", ";
  std::cout << poly[0](1) << "])" << std::endl;
}


void
MortarSegmentHelper::plotTriangulation(std::vector<Point> & nodes, std::vector<std::array<int, 3>> & elem_to_nodes)
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
MortarSegmentHelper::triangulatePoly(std::vector<Point> & nodes, std::vector<std::array<int, 3>> & tri_map)
{
  // Initialize output map
  tri_map.clear();

  // If fewer than 3 nodes, error
  if (nodes.size() < 3)
  {
    mooseError("Can't triangulate poly with fewer than 3 nodes");
  }
  // If three nodes, already a triangle, simply pass back map
  else if (nodes.size() == 3)
  {
    tri_map.push_back({{0,1,2}});
    return;
  }
  // Otherwise add center point node and make simple triangulation
  else
  {
    Point poly_center;
    int n_verts = nodes.size();

    // Get geometric center of polygon
    for (auto node : nodes)
      poly_center += node;
    poly_center /= n_verts;

    // Add triangles formed by outer edge and center point
    for (auto i : make_range(n_verts))
      tri_map.push_back({{i, (i + 1) % n_verts, n_verts}});

    // Add center point to end nodes
    nodes.push_back(poly_center);
    return;
  }
}


void
MortarSegmentHelper::getMortarSegments(const Elem * primary_elem_ptr, std::vector<Point> & nodes, std::vector<std::array<int, 3>> & elem_to_nodes)
{
  nodes.clear();

  // Clip primary elem against secondary elem
  std::vector<Point> clipped_poly;
  clipPoly(primary_elem_ptr, clipped_poly);
  if (clipped_poly.size() < 3)
  {
    elem_to_nodes.clear();
    return;
  }

  if(_debug)
  {
    for(auto pt : clipped_poly)
    {
      if (!isInsideSecondary(pt))
        mooseError("Clipped polygon not inside linearized secondary element");
    }
  }

  // Triangulate clip polygon
  triangulatePoly(clipped_poly, elem_to_nodes);

  // Transform clipped poly back to (linearized) 3d and output nodes
  nodes.reserve(clipped_poly.size());
  for (auto pt : clipped_poly)
    nodes.emplace_back((pt(0) * _u) + (pt(1) * _v) + _center);
}
