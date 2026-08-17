//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurfaceDelaunayGeneratorBase.h"

#include "CrossFieldSolver.h"
#include "IncrementalDelaunay.h"
#include "MeshTriangulationUtils.h"

#include "libmesh/mesh_triangle_holes.h"
#include "libmesh/parsed_function.h"
#include "libmesh/point_locator_base.h"

#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

/**
 * Generates a triangulation in the XY plane by advancing a front, based on an input mesh defining
 * the outer boundary and an optional set of input meshes defining inner hole boundaries.
 *
 * Where XYDelaunayGenerator refines a triangulation until every triangle is small enough, this
 * generator grows one: the triangles that already meet the target size are separated from those
 * that do not by a front, and a point is placed a target size ahead of the worst front edge until
 * nothing is left to place. The triangulation itself is maintained by IncrementalDelaunay, which
 * keeps the outer boundary and the hole boundaries as constrained segments, so the front never
 * crosses them.
 *
 * Points placed under the LINF metric form right isosceles triangles in a local frame rather than
 * equilateral ones, which is what lets pairs of them recombine into good quadrilaterals; the frame
 * comes either from a cross field solved over the domain or from the nearest boundary segment.
 *
 * The result is a TRI3 mesh. Chain SmoothMeshGenerator after this generator to smooth it, and
 * TriToQuadGenerator to recombine it into quadrilaterals.
 */
class XYFrontalDelaunayGenerator : public SurfaceDelaunayGeneratorBase
{
public:
  static InputParameters validParams();

  XYFrontalDelaunayGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /**
   * An edge of the front, which separates the triangles that meet the target size from the
   * triangles that do not.
   */
  struct FrontEdge
  {
    /// Vertex at the start of the edge, which has the triangle that misses the target on its left
    std::size_t start;
    /// Vertex at the end of the edge
    std::size_t end;
    /// Circumradius of that triangle over the target circumradius, larger being further off
    Real excess;
  };

  /**
   * Resolves the boundary and subdomain names that select the outer boundary into the ids that
   * define it, the same ids that XYDelaunayGenerator hands the Delaunay triangulator.
   * @param boundary_mesh The input mesh holding the outer boundary
   * @return The ids selecting the outer boundary, empty when the whole input mesh defines it
   */
  std::set<std::size_t> outerBoundaryIds(MeshBase & boundary_mesh) const;

  /**
   * Triangulates the domain with the existing Delaunay triangulator at _background_area_factor
   * times the requested area. The result carries the cross field and, where no area limit is given,
   * the target area itself, so it only has to cover the domain and be coarse.
   * @param boundary_mesh The input mesh holding the outer boundary
   * @param holes The input meshes holding the holes
   * @param opts The triangulation options of this generator
   * @return The background triangulation
   */
  std::unique_ptr<MeshBase>
  buildBackgroundMesh(const MeshBase & boundary_mesh,
                      const std::vector<std::unique_ptr<MeshBase>> & holes,
                      const MeshTriangulationUtils::XYDelaunayOptions & opts);

  /**
   * @param mesh The background triangulation
   * @return The boundary tangent angle at every boundary node of the background mesh, keyed by node
   * id, in the form the cross field solver takes its Dirichlet data
   */
  static std::map<dof_id_type, Real> boundaryTangentAngles(const MeshBase & mesh);

  /**
   * Adds one closed loop of the input boundary to the points the triangulation is seeded with and
   * to the segments it is constrained by, splitting each segment of the loop into pieces that the
   * front does not have to refine further, and recording which boundary those pieces belong to.
   * @param loop The corners of the loop, in order
   * @param refine Whether segments longer than the local target size may be split
   * @param extra_nodes How many nodes to add inside every segment of the loop
   * @param bcid The boundary id to set on the sides of the output mesh that lie on this loop
   * @param points Appended with the points of the loop
   * @param segments Appended with the segments between them
   */
  void appendLoop(const std::vector<Point> & loop,
                  bool refine,
                  unsigned int extra_nodes,
                  boundary_id_type bcid,
                  std::vector<Point> & points,
                  std::vector<IncrementalDelaunay::Segment> & segments);

  /// @return The target triangle area at a point, from whichever area limit the user set
  Real targetArea(const Point & point) const;

  /// @return The target edge length of a triangle of the given area under the selected metric
  Real targetSize(Real area) const;

  /// @return The circumradius of the target triangle of the given edge length
  Real targetCircumradius(Real size) const;

  /// @return The bucket of the boundary segment grid a point falls in
  std::pair<long, long> boundaryGridKey(const Point & point) const;

  /**
   * Sorts the boundary segments into the buckets the BOUNDARY frame searches, so that a search only
   * has to reach out until it has covered the distance to the nearest segment rather than measure
   * against every segment of the boundary. Only for that frame, and only once the whole boundary
   * has been seeded.
   */
  void buildBoundarySegmentGrid();

  /// @return The local frame (u, v) the LINF metric measures in, at a point
  std::pair<Point, Point> localFrame(const Point & point) const;

  /// @return The distance between two points under the selected metric, measured in the frame
  Real metricDistance(const Point & first,
                      const Point & second,
                      const std::pair<Point, Point> & frame) const;

  /// @return Whether a point lies inside the outer boundary and outside every hole
  bool insideDomain(const Point & point) const;

  /// @return The bucket of the vertex grid a point falls in
  std::pair<long, long> gridKey(const Point & point) const;

  /**
   * Records a vertex in the grid the rejection rule searches.
   * @param vertex The vertex id
   * @param point The position of that vertex
   */
  void addToGrid(std::size_t vertex, const Point & point);

  /**
   * @param delaunay The triangulation holding the vertices
   * @param point The candidate point
   * @param distance The distance no vertex may be within
   * @param frame The frame the LINF metric measures in
   * @return Whether a vertex of the triangulation lies within distance of the point
   */
  bool hasVertexWithin(const IncrementalDelaunay & delaunay,
                       const Point & point,
                       Real distance,
                       const std::pair<Point, Point> & frame) const;

  /**
   * Computes where the advance would place a point ahead of a front edge and applies the rejection
   * rule to it.
   * @param delaunay The triangulation the front belongs to
   * @param edge The front edge to advance
   * @param point Filled with the point to insert
   * @return Whether that point is to be inserted
   */
  bool
  placePoint(const IncrementalDelaunay & delaunay, const FrontEdge & edge, Point & point) const;

  /**
   * @param delaunay The triangulation the front belongs to
   * @param triangles The triangles of that triangulation
   * @param inside Whether each of those triangles lies in the domain
   * @return The front edges, worst first
   */
  std::vector<FrontEdge> collectFront(const IncrementalDelaunay & delaunay,
                                      const std::vector<IncrementalDelaunay::Triangle> & triangles,
                                      const std::vector<bool> & inside) const;

  /**
   * Moves the boundary id recorded for the constrained segment an insertion split onto the two
   * halves that insertion divided it into, which are the segments the output mesh has sides on now.
   * @param delaunay The triangulation, whose constrained segments already hold the two halves
   * @param vertex The vertex the insertion placed on the segment
   */
  void recordSplitBoundaryIds(const IncrementalDelaunay & delaunay, std::size_t vertex);

  /// Advances the front over the whole domain, inserting points into the triangulation
  void advanceFront(IncrementalDelaunay & delaunay);

  /// @return A TRI3 mesh of the triangles of the triangulation that lie in the domain
  std::unique_ptr<MeshBase> buildTriangleMesh(const IncrementalDelaunay & delaunay);

  /// Input mesh defining the boundary to triangulate within
  std::unique_ptr<MeshBase> & _bdy_ptr;

  /// Holds pointers to the pointers to input meshes defining holes
  const std::vector<std::unique_ptr<MeshBase> *> _hole_ptrs;

  /// How many more nodes to add in each outer boundary segment
  const unsigned int _add_nodes_per_boundary_segment;

  /// Whether to allow automatically refining the outer boundary
  const bool _refine_bdy;

  /// Whether to stitch to the mesh defining each hole
  const std::vector<bool> _stitch_holes;

  /// Whether to allow automatically refining each hole boundary
  const std::vector<bool> _refine_holes;

  /// Desired (maximum) triangle area
  const Real _desired_area;

  /// Desired triangle area as a (fparser-compatible) function of x,y
  const std::string _desired_area_func;

  /// Desired interior node locations
  const std::vector<Point> _interior_points;

  /// Norm the target size is measured in when a point is placed ahead of the front
  const MooseEnum _metric;

  /// Where the local frame the LINF metric measures in comes from
  const MooseEnum _orientation;

  /// Outline of the outer boundary, which the advance stays inside
  std::unique_ptr<libMesh::TriangulatorInterface::MeshedHole> _outer_outline;

  /// Outlines of the holes, which the advance stays outside
  std::vector<std::unique_ptr<libMesh::TriangulatorInterface::MeshedHole>> _hole_outlines;

  /// Coarse triangulation of the domain the cross field is solved on
  std::unique_ptr<MeshBase> _background_mesh;

  /// Locates the background element whose area is the target where no area limit was given
  std::unique_ptr<libMesh::PointLocatorBase> _background_locator;

  /// Mean area of the background elements, the target where the background locator finds nothing
  Real _background_mean_area;

  /// Desired area as a function of position, built only when 'desired_area_func' is set
  std::unique_ptr<libMesh::ParsedFunction<Real>> _area_function;

  /// Cross field over the domain, built only when the LINF metric asks for the CROSS_FIELD frame
  std::unique_ptr<CrossFieldSolver> _cross_field;

  /// Segments of the outer boundary and of the holes, whose tangents give the BOUNDARY frame
  std::vector<std::pair<Point, Point>> _boundary_segments;

  /// Side of the buckets the boundary segments are sorted into so that the frame search stays local
  Real _boundary_cell;

  /// Indices into _boundary_segments, bucketed by the cells each of those segments passes through
  std::map<std::pair<long, long>, std::vector<std::size_t>> _boundary_segment_grid;

  /// Side of the buckets the vertices are sorted into so that the rejection rule stays local
  Real _grid_cell;

  /// Vertex ids of the triangulation, bucketed by position
  std::map<std::pair<long, long>, std::vector<std::size_t>> _vertex_grid;

  /// Boundary id of each seed segment, keyed on its two vertices with the smaller id first
  std::map<IncrementalDelaunay::Segment, boundary_id_type> _segment_boundary_ids;

  /// How much coarser in area the background triangulation is than the mesh being generated
  static constexpr Real _background_area_factor = 16.0;

  /// How far the circumradius of a triangle may exceed the target before the advance refines it
  static constexpr Real _size_tolerance = 1.2;

  /// How close to an existing vertex, as a fraction of the target size, a new point may not come
  static constexpr Real _rejection_factor = 0.7;
};
