//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolyhedraUtils.h"

#include "MooseError.h"
#include "MooseUtils.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/boundary_info.h"
#include "libmesh/cell_c0polyhedron.h"
#include "libmesh/face_c0polygon.h"

#include <algorithm>
#include <limits>
#include <map>

using namespace libMesh;

namespace PolyhedraUtils
{
namespace
{
  using Face = FaceDescriptor;

  struct EdgeKey
  {
    unsigned int a, b;
    bool operator<(const EdgeKey & other) const
    {
      return (a < other.a) || (a == other.a && b < other.b);
    }
  };

  struct EdgeFaces
  {
    unsigned int f0;
    unsigned int f1;
  };

  constexpr unsigned int invalid_uint = std::numeric_limits<unsigned int>::max();

  enum class HalfSpaceSide : short
  {
    NEG = -1,
    ON  =  0,
    POS =  1
  };

  HalfSpaceSide
  sideOf(const Point & p, const Point & n, const Real d)
  {
    const Real val = n * p - d;
    if (MooseUtils::absoluteFuzzyEqual(val, 0.0))
      return HalfSpaceSide::ON;
    return (val > 0.0) ? HalfSpaceSide::POS : HalfSpaceSide::NEG;
  }

  Point
  polygonNormal(const Face & f, const std::vector<Point> & coords)
  {
    if (f.vertices.size() < 3)
      return Point(0, 0, 0);

    Point n(0, 0, 0);
    const auto nverts = f.vertices.size();
    for (unsigned int i = 0; i < nverts; ++i)
    {
      const Point & p0 = coords[f.vertices[i]];
      const Point & p1 = coords[f.vertices[(i + 1) % nverts]];
      n += p0.cross(p1);
    }
    const Real norm = n.norm();
    if (MooseUtils::absoluteFuzzyEqual(norm, 0.0))
      return Point(0, 0, 0);
    return n / norm;
  }

  Point
  polygonCentroid(const Face & f, const std::vector<Point> & coords)
  {
    Point c(0, 0, 0);
    if (f.vertices.empty())
      return c;

    for (const auto vi : f.vertices)
      c += coords[vi];
    c /= static_cast<Real>(f.vertices.size());
    return c;
  }

  void
  buildAdjacencyAndNormals(const std::vector<Point> & coords,
                           std::vector<Face> & faces,
                           std::map<EdgeKey, EdgeFaces> & edges,
                           std::vector<Point> & fnormal)
  {
    Point c(0, 0, 0);
    if (!coords.empty())
    {
      for (const auto & p : coords)
        c += p;
      c /= static_cast<Real>(coords.size());
    }

    const unsigned int nfaces = faces.size();
    fnormal.resize(nfaces);

    for (unsigned int fi = 0; fi < nfaces; ++fi)
    {
      Face & f = faces[fi];
      auto n = polygonNormal(f, coords);
      if (MooseUtils::absoluteFuzzyEqual(n.norm(), 0.0))
      {
        fnormal[fi] = n;
        continue;
      }
      const auto fc = polygonCentroid(f, coords);

      if ((fc - c) * n < 0.0)
      {
        std::reverse(f.vertices.begin(), f.vertices.end());
        n = -n;
      }

      fnormal[fi] = n;

      const auto nverts = f.vertices.size();
      for (unsigned int i = 0; i < nverts; ++i)
      {
        auto a = f.vertices[i];
        auto b = f.vertices[(i + 1) % nverts];
        if (a > b)
          std::swap(a, b);

        EdgeKey key{a, b};
        auto it = edges.find(key);
        if (it == edges.end())
          edges.emplace(key, EdgeFaces{fi, invalid_uint});
        else
        {
          if (it->second.f1 == invalid_uint)
            it->second.f1 = fi;
        }
      }
    }
  }

  bool
  findConcaveEdge(const std::map<EdgeKey, EdgeFaces> & edges,
                  const std::vector<Point> & fnormal,
                  EdgeKey & concave_edge,
                  unsigned int & f0,
                  unsigned int & f1)
  {
    for (const auto & kv : edges)
    {
      const auto & key = kv.first;
      const auto & ef  = kv.second;
      if (ef.f0 == invalid_uint || ef.f1 == invalid_uint)
        continue;

      const Point & n0 = fnormal[ef.f0];
      const Point & n1 = fnormal[ef.f1];
      if (MooseUtils::absoluteFuzzyEqual(n0.norm(), 0.0) ||
          MooseUtils::absoluteFuzzyEqual(n1.norm(), 0.0))
        continue;

      const Real dot = n0 * n1;
      if (dot < 0.0)
      {
        concave_edge = key;
        f0 = ef.f0;
        f1 = ef.f1;
        return true;
      }
    }
    return false;
  }

  void
  computeEdgeIntersections(const std::map<EdgeKey, EdgeFaces> & edges,
                           std::vector<Point> & coords,
                           std::map<EdgeKey, unsigned int> & edge_split_vertex,
                           const Point & n_split,
                           const Real d_split)
  {
    edge_split_vertex.clear();

    for (const auto & kv : edges)
    {
      const auto & key = kv.first;
      const Point & p0 = coords[key.a];
      const Point & p1 = coords[key.b];

      const auto s0 = sideOf(p0, n_split, d_split);
      const auto s1 = sideOf(p1, n_split, d_split);

      if ((s0 == HalfSpaceSide::POS && s1 == HalfSpaceSide::NEG) ||
          (s0 == HalfSpaceSide::NEG && s1 == HalfSpaceSide::POS))
      {
        const Real v0 = n_split * p0 - d_split;
        const Real v1 = n_split * p1 - d_split;
        const Real denom = v0 - v1;
        if (MooseUtils::absoluteFuzzyEqual(denom, 0.0))
          continue;
        const Real t = v0 / denom;
        Point pint = p0 + t * (p1 - p0);

        const unsigned int new_index = coords.size();
        coords.push_back(pint);
        edge_split_vertex.emplace(key, new_index);
      }
    }
  }

  void
  clipFaceByPlane(const Face & in_face,
                  const std::vector<Point> & coords,
                  const Point & n_split,
                  const Real d_split,
                  const std::map<EdgeKey, unsigned int> & edge_split_vertex,
                  Face & pos_face,
                  Face & neg_face)
  {
    pos_face.vertices.clear();
    neg_face.vertices.clear();
    pos_face.orig_side = in_face.orig_side;
    neg_face.orig_side = in_face.orig_side;

    const auto & v = in_face.vertices;
    const unsigned int m = v.size();
    if (m < 3)
      return;

    unsigned int prev = v[m - 1];
    auto s_prev = sideOf(coords[prev], n_split, d_split);

    for (unsigned int i = 0; i < m; ++i)
    {
      unsigned int curr = v[i];
      auto s_curr = sideOf(coords[curr], n_split, d_split);

      unsigned int a = prev, b = curr;
      if (a > b)
        std::swap(a, b);
      EdgeKey key{a, b};
      const auto it_split = edge_split_vertex.find(key);
      const bool has_split = (it_split != edge_split_vertex.end());
      const unsigned int v_split = has_split ? it_split->second : invalid_uint;

      if (s_prev == HalfSpaceSide::POS || s_prev == HalfSpaceSide::ON)
        pos_face.vertices.push_back(prev);
      if (s_prev == HalfSpaceSide::NEG || s_prev == HalfSpaceSide::ON)
        neg_face.vertices.push_back(prev);

      if (has_split)
      {
        pos_face.vertices.push_back(v_split);
        neg_face.vertices.push_back(v_split);
      }

      prev = curr;
      s_prev = s_curr;
    }
  }

  Elem *
  buildPolyhedronFromFaces(ReplicatedMesh & mesh,
                           const std::vector<Point> & coords,
                           std::vector<const Node *> & nodes,
                           const std::vector<Face> & faces,
                           const subdomain_id_type new_subdomain_id)
  {
    if (faces.size() < 4)
      return nullptr;

    for (unsigned int i = 0; i < nodes.size(); ++i)
      if (!nodes[i])
        nodes[i] = mesh.add_point(coords[i]);

    std::vector<std::shared_ptr<Polygon>> sides;
    sides.reserve(faces.size());
    for (const auto & f : faces)
    {
      if (f.vertices.size() < 3)
        continue;
      auto poly = std::make_shared<C0Polygon>(f.vertices.size());
      for (unsigned int i = 0; i < f.vertices.size(); ++i)
        poly->set_node(i, const_cast<Node *>(nodes[f.vertices[i]]));
      sides.push_back(poly);
    }

    if (sides.size() < 4)
      return nullptr;

    std::unique_ptr<Node> mid_elem_node;
    auto poly = std::make_unique<C0Polyhedron>(sides, mid_elem_node);

    poly->subdomain_id() = new_subdomain_id;
    Elem * new_elem = mesh.add_elem(std::move(poly));
    if (mid_elem_node)
      mesh.add_node(std::move(mid_elem_node));

    return new_elem;
  }

} // anonymous namespace

bool
splitNonConvexPolyhedron(ReplicatedMesh & mesh,
                         Elem * orig_elem,
                         const std::vector<const Node *> & existing_nodes,
                         const std::vector<Face> & faces_in,
                         const std::vector<std::vector<boundary_id_type>> & elem_side_list,
                         const subdomain_id_type sid_shift_base,
                         const boundary_id_type cut_face_id)
{
  if (!orig_elem)
    mooseError("splitNonConvexPolyhedron called with null orig_elem");

  if (existing_nodes.empty() || faces_in.size() < 4)
    return false;

  std::vector<Point> coords;
  coords.reserve(existing_nodes.size());
  std::vector<const Node *> nodes;
  nodes.reserve(existing_nodes.size());

  for (const auto * n : existing_nodes)
  {
    coords.push_back(static_cast<Point>(*n));
    nodes.push_back(n);
  }

  std::vector<Face> faces = faces_in;

  std::map<EdgeKey, EdgeFaces> edges;
  std::vector<Point> fnormal;
  buildAdjacencyAndNormals(coords, faces, edges, fnormal);

  EdgeKey ce{0u, 0u};
  unsigned int f0 = invalid_uint, f1 = invalid_uint;
  if (!findConcaveEdge(edges, fnormal, ce, f0, f1))
    return false;

  Point n_split = fnormal[f0] + fnormal[f1];
  const Real n_norm = n_split.norm();
  if (MooseUtils::absoluteFuzzyEqual(n_norm, 0.0))
    n_split = fnormal[f0];
  else
    n_split /= n_norm;

  const Point p0 = coords[ce.a];
  const Point p1 = coords[ce.b];
  const Point p_mid = 0.5 * (p0 + p1);
  const Real d_split = n_split * p_mid;

  std::map<EdgeKey, unsigned int> edge_split_vertex;
  computeEdgeIntersections(edges, coords, edge_split_vertex, n_split, d_split);

  std::vector<Face> faces_pos, faces_neg;
  faces_pos.reserve(faces.size());
  faces_neg.reserve(faces.size());

  Face tmp_pos, tmp_neg;
  for (const auto & f : faces)
  {
    clipFaceByPlane(f, coords, n_split, d_split, edge_split_vertex, tmp_pos, tmp_neg);

    if (tmp_pos.vertices.size() >= 3)
      faces_pos.push_back(tmp_pos);
    if (tmp_neg.vertices.size() >= 3)
      faces_neg.push_back(tmp_neg);
  }

  if (faces_pos.empty() || faces_neg.empty())
    return false;

  const auto new_sid = orig_elem->subdomain_id() + sid_shift_base;
  Elem * pos_elem = buildPolyhedronFromFaces(mesh, coords, nodes, faces_pos, new_sid);
  Elem * neg_elem = buildPolyhedronFromFaces(mesh, coords, nodes, faces_neg, new_sid);

  if (!pos_elem || !neg_elem)
    return false;

  auto & boundary_info = mesh.get_boundary_info();

  auto reattach_boundaries = [&](Elem * new_elem, const std::vector<Face> & child_faces)
  {
    for (unsigned int new_side_i = 0; new_side_i < child_faces.size(); ++new_side_i)
    {
      const auto & f = child_faces[new_side_i];
      const int orig_side = f.orig_side;

      if (orig_side < 0)
      {
        // Faces lying on the original cut interface have orig_side == -1.
        // Internal split faces use other negative values (currently none),
        // so we only attach cut_face_id for orig_side == -1.
        if (orig_side == -1)
          boundary_info.add_side(new_elem, new_side_i, cut_face_id);
      }
      else
      {
        if (static_cast<unsigned int>(orig_side) >= elem_side_list.size())
          continue;
        const auto & bdrys = elem_side_list[orig_side];
        for (const auto bid : bdrys)
          boundary_info.add_side(new_elem, new_side_i, bid);
      }
    }
  };

  reattach_boundaries(pos_elem, faces_pos);
  reattach_boundaries(neg_elem, faces_neg);

  return true;
}

} // namespace PolyhedraUtils
