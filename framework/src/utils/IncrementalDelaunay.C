//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IncrementalDelaunay.h"

#include "MooseError.h"
#include "predicates.h"

#include "libmesh/int_range.h"

#include <algorithm>
#include <map>

namespace
{
static_assert(sizeof(IncrementalDelaunay::Point2D) == 2 * sizeof(double),
              "The exact predicates read a point as two adjacent doubles.");

/// @return the two coordinates of a point, in the layout the exact predicates take
const double *
coords(const IncrementalDelaunay::Point2D & p)
{
  return &p.x;
}
}

std::size_t
IncrementalDelaunay::numPoints() const
{
  return _vertices.empty() ? 0 : _vertices.size() - _num_bounding;
}

std::size_t
IncrementalDelaunay::toInternal(const std::size_t id) const
{
  if (id >= numPoints())
    mooseError("IncrementalDelaunay: vertex id ",
               id,
               " does not exist; the triangulation has ",
               numPoints(),
               " points.");
  return id + _num_bounding;
}

std::size_t
IncrementalDelaunay::toCaller(const std::size_t v) const
{
  mooseAssert(!isBounding(v), "A bounding triangle vertex has no caller vertex id");
  return v - _num_bounding;
}

const IncrementalDelaunay::Point2D &
IncrementalDelaunay::point(const std::size_t id) const
{
  return _vertices[toInternal(id)];
}

IncrementalDelaunay::Segment
IncrementalDelaunay::makeSegment(const std::size_t v0, const std::size_t v1)
{
  return {std::min(v0, v1), std::max(v0, v1)};
}

bool
IncrementalDelaunay::isConstrainedSegment(const std::size_t v0, const std::size_t v1) const
{
  return _constraints.count(makeSegment(v0, v1)) > 0;
}

bool
IncrementalDelaunay::isConstrainedEdge(const std::size_t v0, const std::size_t v1) const
{
  if (isBounding(v0) || isBounding(v1))
    return false;
  return _constraints.count(makeSegment(toCaller(v0), toCaller(v1))) > 0;
}

unsigned int
IncrementalDelaunay::localVertexIndex(const std::size_t t, const std::size_t v) const
{
  for (const auto i : make_range(3u))
    if (_triangles[t].vertices[i] == v)
      return i;
  mooseError("IncrementalDelaunay: triangle ", t, " does not have vertex ", v, ".");
}

unsigned int
IncrementalDelaunay::localEdgeIndex(const std::size_t t, const Segment & edge) const
{
  for (const auto i : make_range(3u))
    if (makeSegment(_triangles[t].vertices[(i + 1) % 3], _triangles[t].vertices[(i + 2) % 3]) ==
        edge)
      return i;
  mooseError("IncrementalDelaunay: triangle ",
             t,
             " does not have an edge between vertices ",
             edge.first,
             " and ",
             edge.second,
             ".");
}

bool
IncrementalDelaunay::containsPoint(const std::size_t t, const Point2D & p) const
{
  for (const auto i : make_range(3u))
    if (orient2d(xy(_triangles[t].vertices[(i + 1) % 3]),
                 xy(_triangles[t].vertices[(i + 2) % 3]),
                 coords(p)) < 0.0)
      return false;
  return true;
}

bool
IncrementalDelaunay::isStrictlyBetween(const std::size_t v_first,
                                       const std::size_t v_mid,
                                       const std::size_t v_last) const
{
  // The three are collinear, so whichever coordinate separates the two ends orders all three.
  const auto & first = _vertices[v_first];
  const auto & mid = _vertices[v_mid];
  const auto & last = _vertices[v_last];
  if (first.x != last.x)
    return (first.x < mid.x && mid.x < last.x) || (last.x < mid.x && mid.x < first.x);
  return (first.y < mid.y && mid.y < last.y) || (last.y < mid.y && mid.y < first.y);
}

std::string
IncrementalDelaunay::vertexName(const std::size_t v) const
{
  if (isBounding(v))
    return "bounding vertex " + std::to_string(v);
  return "point " + std::to_string(toCaller(v));
}

std::size_t
IncrementalDelaunay::locate(const Point2D & p) const
{
  auto current = _last_triangle < _triangles.size() ? _last_triangle : std::size_t(0);

  // The walk always arrives in a Delaunay triangulation, but a constrained one can in principle
  // send it round in a circle, so it gives up and scans rather than spin.
  auto steps_left = 2 * _triangles.size() + 8;
  while (steps_left > 0)
  {
    --steps_left;

    auto next = invalid_index;
    bool inside = true;
    for (const auto i : make_range(3u))
      if (orient2d(xy(_triangles[current].vertices[(i + 1) % 3]),
                   xy(_triangles[current].vertices[(i + 2) % 3]),
                   coords(p)) < 0.0)
      {
        inside = false;
        next = _triangles[current].neighbors[i];
        break;
      }

    if (inside)
      return current;
    if (next == invalid_index)
      break;
    current = next;
  }

  for (const auto t : index_range(_triangles))
    if (containsPoint(t, p))
      return t;
  return invalid_index;
}

void
IncrementalDelaunay::growCavity(const std::size_t seed,
                                const std::size_t v_new,
                                std::set<std::size_t> & cavity) const
{
  cavity.insert(seed);
  std::vector<std::size_t> pending{seed};

  while (!pending.empty())
  {
    const auto t = pending.back();
    pending.pop_back();

    for (const auto i : make_range(3u))
    {
      const auto v0 = _triangles[t].vertices[(i + 1) % 3];
      const auto v1 = _triangles[t].vertices[(i + 2) % 3];
      const auto side = orient2d(xy(v0), xy(v1), xy(v_new));

      if (isConstrainedEdge(v0, v1))
      {
        if (side <= 0.0)
          mooseError("IncrementalDelaunay: the point (",
                     _vertices[v_new].x,
                     ", ",
                     _vertices[v_new].y,
                     ") is on or beyond constrained segment (",
                     toCaller(v0),
                     ", ",
                     toCaller(v1),
                     "), so it lies outside the region that segment bounds.");
        continue;
      }

      const auto n = _triangles[t].neighbors[i];
      if (n == invalid_index || cavity.count(n) > 0)
        continue;

      // Taking the neighbor in when the shared edge would give a triangle of zero or negative area
      // is what keeps the cavity star shaped about the new vertex; with exact predicates that only
      // happens when the new vertex falls exactly on the edge.
      if (side <= 0.0 || incircle(xy(_triangles[n].vertices[0]),
                                  xy(_triangles[n].vertices[1]),
                                  xy(_triangles[n].vertices[2]),
                                  xy(v_new)) > 0.0)
      {
        cavity.insert(n);
        pending.push_back(n);
      }
    }
  }
}

std::vector<std::size_t>
IncrementalDelaunay::retriangulate(const std::vector<std::size_t> & removed,
                                   const std::vector<std::array<std::size_t, 3>> & added)
{
  mooseAssert(added.size() >= removed.size(),
              "A region takes at least as many triangles to cover as it is emptied of");

  const std::set<std::size_t> emptied(removed.begin(), removed.end());

  // What lies outside the region being replaced, keyed on the edge it is joined along.
  std::map<Segment, std::size_t> outside;
  for (const auto t : emptied)
    for (const auto i : make_range(3u))
    {
      const auto n = _triangles[t].neighbors[i];
      if (n != invalid_index && emptied.count(n) == 0)
        outside.emplace(
            makeSegment(_triangles[t].vertices[(i + 1) % 3], _triangles[t].vertices[(i + 2) % 3]),
            n);
    }

  // Filling the emptied slots in id order, rather than in the order the region happened to be
  // walked, is what makes the triangle numbering repeat from one run to the next.
  std::vector<std::size_t> slots(emptied.begin(), emptied.end());
  while (slots.size() < added.size())
  {
    slots.push_back(_triangles.size());
    _triangles.emplace_back();
  }

  for (const auto i : index_range(added))
  {
    auto & triangle = _triangles[slots[i]];
    triangle.vertices = added[i];
    triangle.neighbors = {invalid_index, invalid_index, invalid_index};
    for (const auto v : triangle.vertices)
      _vertex_triangle[v] = slots[i];
  }

  // An edge two new triangles share joins them to each other; one only a single new triangle has is
  // on the boundary of the region and joins it to what was already outside.
  std::map<Segment, std::pair<std::size_t, unsigned int>> open_edges;
  for (const auto i : index_range(added))
  {
    const auto t = slots[i];
    for (const auto e : make_range(3u))
    {
      const auto edge =
          makeSegment(_triangles[t].vertices[(e + 1) % 3], _triangles[t].vertices[(e + 2) % 3]);
      const auto it = open_edges.find(edge);
      if (it == open_edges.end())
        open_edges.emplace(edge, std::make_pair(t, e));
      else
      {
        const auto [other_t, other_e] = it->second;
        _triangles[t].neighbors[e] = other_t;
        _triangles[other_t].neighbors[other_e] = t;
        open_edges.erase(it);
      }
    }
  }

  for (const auto & [edge, entry] : open_edges)
  {
    const auto [t, e] = entry;
    const auto it = outside.find(edge);
    if (it == outside.end())
      continue;
    _triangles[t].neighbors[e] = it->second;
    _triangles[it->second].neighbors[localEdgeIndex(it->second, edge)] = t;
  }

  return slots;
}

void
IncrementalDelaunay::triangulatePseudopolygon(
    const std::size_t v_start,
    const std::size_t v_end,
    const std::vector<std::size_t> & chain,
    const std::size_t first,
    const std::size_t last,
    std::vector<std::array<std::size_t, 3>> & triangles) const
{
  if (first == last)
    return;

  auto best = first;
  for (const auto i : make_range(first + 1, last))
    if (incircle(xy(v_start), xy(v_end), xy(chain[best]), xy(chain[i])) > 0.0)
      best = i;

  triangles.push_back({v_start, v_end, chain[best]});
  triangulatePseudopolygon(chain[best], v_end, chain, first, best, triangles);
  triangulatePseudopolygon(v_start, chain[best], chain, best + 1, last, triangles);
}

void
IncrementalDelaunay::initialize(const std::vector<Point2D> & points,
                                const std::vector<Segment> & segments)
{
  initPredicates();

  if (points.empty())
    mooseError("IncrementalDelaunay: initialize() needs at least one point.");

  _vertices.clear();
  _triangles.clear();
  _vertex_triangle.clear();
  _constraints.clear();

  auto x_min = points.front().x;
  auto x_max = x_min;
  auto y_min = points.front().y;
  auto y_max = y_min;
  for (const auto & p : points)
  {
    x_min = std::min(x_min, p.x);
    x_max = std::max(x_max, p.x);
    y_min = std::min(y_min, p.y);
    y_max = std::max(y_max, p.y);
  }

  // A triangle far enough outside the points that all of them are strictly inside it, which is what
  // makes the cavity of every insertion a closed polygon.
  const auto reach = _bounding_reach * std::max({x_max - x_min, y_max - y_min, 1.0});
  const auto x_mid = 0.5 * (x_min + x_max);
  const auto y_mid = 0.5 * (y_min + y_max);
  _vertices.push_back({x_mid - reach, y_mid - reach});
  _vertices.push_back({x_mid + reach, y_mid - reach});
  _vertices.push_back({x_mid, y_mid + reach});
  _vertex_triangle.assign(_num_bounding, std::size_t(0));

  Triangle bounding;
  bounding.vertices = {0, 1, 2};
  bounding.neighbors = {invalid_index, invalid_index, invalid_index};
  _triangles.push_back(bounding);
  _last_triangle = 0;

  for (const auto i : index_range(points))
  {
    const auto id = insertPoint(points[i]);
    if (id != i)
      mooseError("IncrementalDelaunay: points ",
                 id,
                 " and ",
                 i,
                 " are the same point; initialize() needs the points to be distinct, because the "
                 "constrained segments refer to them by position.");
  }

  for (const auto & [v0, v1] : segments)
    insertSegment(v0, v1);
}

std::size_t
IncrementalDelaunay::insertPoint(const Point2D & p)
{
  initPredicates();

  if (_triangles.empty())
    mooseError("IncrementalDelaunay: initialize() has to run before a point can be inserted.");

  const auto seed = locate(p);
  if (seed == invalid_index)
    mooseError("IncrementalDelaunay: the point (",
               p.x,
               ", ",
               p.y,
               ") is outside the region initialize() enclosed, so it cannot be inserted.");

  // Two points count as the same point only when their coordinates agree exactly, which is the
  // criterion the exact predicates use as well.
  for (const auto v : _triangles[seed].vertices)
    if (_vertices[v].x == p.x && _vertices[v].y == p.y)
    {
      if (isBounding(v))
        mooseError("IncrementalDelaunay: the point (",
                   p.x,
                   ", ",
                   p.y,
                   ") is a vertex of the bounding triangle, so it is far outside the region "
                   "initialize() enclosed.");
      return toCaller(v);
    }

  // A point landing on a constrained segment divides it, because the cavity may not cross a
  // constrained segment and a segment with a vertex on it can no longer be a single edge.
  Segment split(invalid_index, invalid_index);
  for (const auto i : make_range(3u))
  {
    const auto v0 = _triangles[seed].vertices[(i + 1) % 3];
    const auto v1 = _triangles[seed].vertices[(i + 2) % 3];
    if (orient2d(xy(v0), xy(v1), coords(p)) == 0.0 && isConstrainedEdge(v0, v1))
    {
      split = makeSegment(toCaller(v0), toCaller(v1));
      _constraints.erase(split);
      break;
    }
  }

  _vertices.push_back(p);
  _vertex_triangle.push_back(invalid_index);
  const auto v_new = _vertices.size() - 1;

  std::set<std::size_t> cavity;
  growCavity(seed, v_new, cavity);

  std::vector<std::array<std::size_t, 3>> added;
  for (const auto t : cavity)
    for (const auto i : make_range(3u))
    {
      const auto n = _triangles[t].neighbors[i];
      if (n == invalid_index || cavity.count(n) == 0)
        added.push_back(
            {_triangles[t].vertices[(i + 1) % 3], _triangles[t].vertices[(i + 2) % 3], v_new});
    }

  mooseAssert(added.size() == cavity.size() + 2,
              "The cavity of an insertion is a disc, whose boundary has two more edges than the "
              "disc has triangles");

  const std::vector<std::size_t> removed(cavity.begin(), cavity.end());
  _last_triangle = retriangulate(removed, added).front();

  const auto id = toCaller(v_new);
  if (split.first != invalid_index)
  {
    insertSegment(split.first, id);
    insertSegment(id, split.second);
  }
  return id;
}

void
IncrementalDelaunay::insertSegment(const std::size_t v0, const std::size_t v1)
{
  initPredicates();

  if (v0 == v1)
    mooseError("IncrementalDelaunay: a constrained segment needs two different vertices, but both "
               "ends of this one are vertex ",
               v0,
               ".");

  const auto v_from = toInternal(v0);
  const auto v_to = toInternal(v1);

  // Turn around v_from until the triangle the segment leaves through is found. The segment is
  // already an edge if v_to is met on the way, and needs splitting if some other vertex is.
  auto entered = invalid_index;
  auto right = invalid_index;
  auto left = invalid_index;
  const auto start = _vertex_triangle[v_from];
  mooseAssert(start < _triangles.size(), "Every vertex records a triangle it belongs to");

  auto current = start;
  do
  {
    const auto i = localVertexIndex(current, v_from);
    const auto next_v = _triangles[current].vertices[(i + 1) % 3];
    const auto far_v = _triangles[current].vertices[(i + 2) % 3];

    if (next_v == v_to)
    {
      _constraints.insert(makeSegment(v0, v1));
      return;
    }

    const auto next_side = orient2d(xy(v_from), xy(next_v), xy(v_to));

    // Nothing has been changed yet, so recovering the two halves from scratch is safe.
    if (next_side == 0.0 && isStrictlyBetween(v_from, next_v, v_to))
    {
      insertSegment(v0, toCaller(next_v));
      insertSegment(toCaller(next_v), v1);
      return;
    }

    if (next_side > 0.0 && orient2d(xy(v_from), xy(far_v), xy(v_to)) < 0.0)
    {
      entered = current;
      right = next_v;
      left = far_v;
      break;
    }

    current = _triangles[current].neighbors[(i + 1) % 3];
  } while (current != start && current != invalid_index);

  if (entered == invalid_index)
    mooseError("IncrementalDelaunay: no triangle around vertex ",
               v0,
               " is entered by the segment to vertex ",
               v1,
               ".");

  // Follow the segment across the triangulation, keeping the vertices it passes on either side.
  std::vector<std::size_t> crossed{entered};
  std::vector<std::size_t> right_chain{right};
  std::vector<std::size_t> left_chain{left};

  while (true)
  {
    if (isConstrainedEdge(right, left))
      mooseError("IncrementalDelaunay: the segment from vertex ",
                 v0,
                 " to vertex ",
                 v1,
                 " crosses constrained segment (",
                 toCaller(right),
                 ", ",
                 toCaller(left),
                 "); constrained segments may only meet at their ends.");

    const auto edge = makeSegment(right, left);
    const auto next_t = _triangles[crossed.back()].neighbors[localEdgeIndex(crossed.back(), edge)];
    if (next_t == invalid_index)
      mooseError("IncrementalDelaunay: the segment from vertex ",
                 v0,
                 " to vertex ",
                 v1,
                 " leaves the triangulation before it reaches its end.");

    const auto apex = _triangles[next_t].vertices[localEdgeIndex(next_t, edge)];
    if (apex == v_to)
    {
      crossed.push_back(next_t);
      break;
    }

    const auto side = orient2d(xy(v_from), xy(v_to), xy(apex));
    if (side == 0.0)
    {
      insertSegment(v0, toCaller(apex));
      insertSegment(toCaller(apex), v1);
      return;
    }

    crossed.push_back(next_t);
    if (side > 0.0)
    {
      left_chain.push_back(apex);
      left = apex;
    }
    else
    {
      right_chain.push_back(apex);
      right = apex;
    }
  }

  // The segment leaves a polygon on either side of itself, every vertex of which the segment sees,
  // so triangulating each on its own restores the Delaunay property everywhere.
  std::vector<std::array<std::size_t, 3>> added;
  const std::vector<std::size_t> left_polygon(left_chain.rbegin(), left_chain.rend());
  triangulatePseudopolygon(v_from, v_to, left_polygon, 0, left_polygon.size(), added);
  triangulatePseudopolygon(v_to, v_from, right_chain, 0, right_chain.size(), added);

  retriangulate(crossed, added);
  _constraints.insert(makeSegment(v0, v1));
}

std::vector<IncrementalDelaunay::Triangle>
IncrementalDelaunay::getTriangles() const
{
  std::vector<std::size_t> position(_triangles.size(), invalid_index);
  std::size_t count = 0;
  for (const auto t : index_range(_triangles))
    if (!isBounding(_triangles[t].vertices[0]) && !isBounding(_triangles[t].vertices[1]) &&
        !isBounding(_triangles[t].vertices[2]))
      position[t] = count++;

  std::vector<Triangle> triangles(count);
  for (const auto t : index_range(_triangles))
  {
    if (position[t] == invalid_index)
      continue;

    auto & out = triangles[position[t]];
    for (const auto i : make_range(3u))
    {
      out.vertices[i] = toCaller(_triangles[t].vertices[i]);
      const auto n = _triangles[t].neighbors[i];
      out.neighbors[i] = n == invalid_index ? invalid_index : position[n];
    }
  }
  return triangles;
}

std::vector<std::string>
IncrementalDelaunay::checkInvariants() const
{
  initPredicates();

  std::vector<std::string> violations;
  std::set<Segment> edges;

  for (const auto t : index_range(_triangles))
  {
    const auto & triangle = _triangles[t];
    if (orient2d(xy(triangle.vertices[0]), xy(triangle.vertices[1]), xy(triangle.vertices[2])) <=
        0.0)
      violations.push_back("triangle " + std::to_string(t) +
                           " is not counter-clockwise, so its area is zero or negative");

    for (const auto i : make_range(3u))
    {
      const auto v0 = triangle.vertices[(i + 1) % 3];
      const auto v1 = triangle.vertices[(i + 2) % 3];
      edges.insert(makeSegment(v0, v1));

      const auto n = triangle.neighbors[i];
      if (n == invalid_index)
        continue;
      if (n >= _triangles.size())
      {
        violations.push_back("triangle " + std::to_string(t) + " has neighbor " +
                             std::to_string(n) + ", which is not a triangle");
        continue;
      }

      unsigned int j = 3;
      for (const auto k : make_range(3u))
        if (makeSegment(_triangles[n].vertices[(k + 1) % 3], _triangles[n].vertices[(k + 2) % 3]) ==
            makeSegment(v0, v1))
          j = k;

      if (j == 3)
      {
        violations.push_back("triangles " + std::to_string(t) + " and " + std::to_string(n) +
                             " are neighbors but share no edge");
        continue;
      }
      if (_triangles[n].neighbors[j] != t)
        violations.push_back("triangle " + std::to_string(t) + " has neighbor " +
                             std::to_string(n) + ", which does not have it back");

      if (!isConstrainedEdge(v0, v1) && incircle(xy(triangle.vertices[0]),
                                                 xy(triangle.vertices[1]),
                                                 xy(triangle.vertices[2]),
                                                 xy(_triangles[n].vertices[j])) > 0.0)
        violations.push_back("the edge between " + vertexName(v0) + " and " + vertexName(v1) +
                             " is not constrained and is not locally Delaunay, because " +
                             vertexName(_triangles[n].vertices[j]) +
                             " is inside the circumcircle of triangle " + std::to_string(t));
    }
  }

  for (const auto & [v0, v1] : _constraints)
    if (edges.count(makeSegment(toInternal(v0), toInternal(v1))) == 0)
      violations.push_back("constrained segment (" + std::to_string(v0) + ", " +
                           std::to_string(v1) + ") is not an edge of the triangulation");

  return violations;
}

std::vector<std::string>
IncrementalDelaunay::checkEmptyCircumcircle() const
{
  initPredicates();

  std::vector<std::string> violations;
  for (const auto t : index_range(_triangles))
  {
    const auto & triangle = _triangles[t];
    for (const auto v : index_range(_vertices))
    {
      if (v == triangle.vertices[0] || v == triangle.vertices[1] || v == triangle.vertices[2])
        continue;
      if (incircle(
              xy(triangle.vertices[0]), xy(triangle.vertices[1]), xy(triangle.vertices[2]), xy(v)) >
          0.0)
        violations.push_back(vertexName(v) + " is inside the circumcircle of triangle " +
                             std::to_string(t));
    }
  }
  return violations;
}
