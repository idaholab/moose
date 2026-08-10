//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "CrossFieldSolver.h"
#include "MooseObjectUnitTest.h"

#include "libmesh/elem.h"
#include "libmesh/face_tri3.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_base.h"
#include "libmesh/node.h"
#include "libmesh/parallel.h"
#include "libmesh/point.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/utility.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

/**
 * The solver builds its own serial communicator and its own equation systems, so the only things it
 * needs from an application are a communicator to build background meshes on and the libMesh and
 * PETSc initialization that constructing the application performs.
 */
class CrossFieldSolverTest : public MooseObjectUnitTest
{
public:
  CrossFieldSolverTest() : MooseObjectUnitTest("MooseUnitApp") {}
};

namespace
{

/// Tolerance on a cross direction the penalty conditions pin directly, which the linear solve
/// resolves several orders of magnitude more tightly than this.
constexpr Real boundary_tol = 1.0e-6;

/// Tolerance on a cross direction away from the boundary, set by the linear interpolation error of
/// a quartic field on a coarse background mesh rather than by the linear solve.
constexpr Real interior_tol = 0.05;

/// Tolerance on quantities that are exact up to round-off, either algebra performed on an already
/// solved field or a coordinate the mesh construction places exactly.
constexpr Real algebraic_tol = 1.0e-12;

/// Radius of the disk background mesh.
constexpr Real disk_radius = 1.0;

/// A background triangulation together with the boundary data the solver needs to go with it.
struct BackgroundMesh
{
  /// The triangulation itself.
  std::unique_ptr<libMesh::ReplicatedMesh> mesh;
  /// Boundary tangent angle of every boundary node of the triangulation, keyed by node id.
  std::map<dof_id_type, Real> boundary_tangent_angles;
};

/// An element of a background mesh that the cross field winds around, which is where a singularity
/// of the field has to sit.
struct WindingSite
{
  /// Vertex average of the element the winding was found on.
  Point center;
  /// Number of times arg(z) winds around the element, counted counter-clockwise.
  int winding;
  /// Whether the solver reports at least one of the element's nodes as singular.
  bool at_singular_node;
};

/// @return @p angle reduced modulo a full turn into [-pi, pi].
Real
wrapToPi(const Real angle)
{
  return angle - 2.0 * libMesh::pi * std::round(angle / (2.0 * libMesh::pi));
}

/**
 * @return the difference between two cross field angles, reduced modulo pi/2 into [-pi/4, pi/4].
 * Crosses are four fold symmetric, so two directions that differ by a quarter turn are the same
 * cross and only this reduced difference is meaningful.
 */
Real
crossAngleDifference(const Real first, const Real second)
{
  return wrapToPi(4.0 * (first - second)) / 4.0;
}

/// @return the three nodes of a TRI3 in counter-clockwise order, whichever order it stores them in.
std::array<const libMesh::Node *, 3>
counterClockwiseNodes(const libMesh::Elem & elem)
{
  std::array<const libMesh::Node *, 3> nodes = {
      elem.node_ptr(0), elem.node_ptr(1), elem.node_ptr(2)};

  const Point first_edge = *nodes[1] - *nodes[0];
  const Point second_edge = *nodes[2] - *nodes[0];
  if (first_edge(0) * second_edge(1) - first_edge(1) * second_edge(0) < 0.0)
    std::swap(nodes[1], nodes[2]);

  return nodes;
}

void
addTri(libMesh::MeshBase & mesh, libMesh::Node * n0, libMesh::Node * n1, libMesh::Node * n2)
{
  libMesh::Elem * elem = mesh.add_elem(new libMesh::Tri3);
  elem->set_node(0, n0);
  elem->set_node(1, n1);
  elem->set_node(2, n2);
}

/**
 * Boundary tangent angle of every boundary node of @p mesh, taken from the boundary edge that
 * leaves the node when the boundary is walked counter-clockwise. The two edges meeting at a corner
 * disagree, and taking the outgoing one makes the choice depend on the geometry alone rather than
 * on the order the elements happen to be visited in.
 * @param mesh a triangulation of a planar domain
 * @return the tangent angle in radians of each boundary node, keyed by node id
 */
std::map<dof_id_type, Real>
outgoingBoundaryTangents(const libMesh::MeshBase & mesh)
{
  // An edge on the boundary belongs to one element, an interior edge to two.
  std::map<std::pair<dof_id_type, dof_id_type>, unsigned int> edge_use_count;
  for (const auto & elem : mesh.element_ptr_range())
    for (const auto i : make_range(3u))
    {
      const auto first = elem->node_id(i);
      const auto second = elem->node_id((i + 1) % 3);
      ++edge_use_count[std::make_pair(std::min(first, second), std::max(first, second))];
    }

  std::map<dof_id_type, Real> tangent_angles;
  for (const auto & elem : mesh.element_ptr_range())
  {
    const auto nodes = counterClockwiseNodes(*elem);
    for (const auto i : make_range(3u))
    {
      const libMesh::Node & from = *nodes[i];
      const libMesh::Node & to = *nodes[(i + 1) % 3];
      const auto key = std::make_pair(std::min(from.id(), to.id()), std::max(from.id(), to.id()));
      if (libmesh_map_find(edge_use_count, key) != 1)
        continue;

      tangent_angles[from.id()] = std::atan2(to(1) - from(1), to(0) - from(0));
    }
  }

  return tangent_angles;
}

/**
 * Background mesh of a disk centered on the origin, built as concentric rings of nodes joined by a
 * fan of triangles at the center and by two triangles per quadrilateral between rings. The boundary
 * tangent is the analytic tangent of the circle rather than the direction of the polygonal edge, so
 * that the recovered field can be compared against the exact solution on the disk.
 * @param comm the communicator to build the mesh on
 * @param n_angular number of nodes per ring, which must exceed eight so that arg(z) advances by
 * less than half a turn between neighboring boundary nodes
 * @param n_radial number of rings, that is, the number of elements along a radius
 */
BackgroundMesh
makeDiskMesh(const libMesh::Parallel::Communicator & comm,
             const unsigned int n_angular,
             const unsigned int n_radial)
{
  BackgroundMesh background;
  background.mesh = std::make_unique<libMesh::ReplicatedMesh>(comm);
  auto & mesh = *background.mesh;
  mesh.set_mesh_dimension(2);
  mesh.set_spatial_dimension(2);

  // The boundary data and every result the solver reports are keyed by node id.
  mesh.allow_renumbering(false);

  libMesh::Node * const center = mesh.add_point(Point(0.0, 0.0, 0.0));

  // ring[j][k] sits at radius (j + 1) * disk_radius / n_radial and angle 2 pi k / n_angular.
  std::vector<std::vector<libMesh::Node *>> ring(n_radial,
                                                 std::vector<libMesh::Node *>(n_angular, nullptr));
  for (const auto j : make_range(n_radial))
    for (const auto k : make_range(n_angular))
    {
      const Real radius = (j + 1) * disk_radius / n_radial;
      const Real angle = 2.0 * libMesh::pi * k / n_angular;
      ring[j][k] = mesh.add_point(Point(radius * std::cos(angle), radius * std::sin(angle), 0.0));
    }

  for (const auto k : make_range(n_angular))
    addTri(mesh, center, ring[0][k], ring[0][(k + 1) % n_angular]);

  for (const auto j : make_range(n_radial - 1))
    for (const auto k : make_range(n_angular))
    {
      const auto next = (k + 1) % n_angular;
      addTri(mesh, ring[j][k], ring[j + 1][k], ring[j + 1][next]);
      addTri(mesh, ring[j][k], ring[j + 1][next], ring[j][next]);
    }

  mesh.prepare_for_use();

  for (const auto k : make_range(n_angular))
  {
    const Real angle = 2.0 * libMesh::pi * k / n_angular;
    background.boundary_tangent_angles[ring[n_radial - 1][k]->id()] = angle + 0.5 * libMesh::pi;
  }

  return background;
}

/**
 * Background mesh of the L shape, the square [0, 2] x [0, 2] with the quadrant [1, 2] x [1, 2]
 * removed so that the reentrant corner sits at (1, 1). Every edge is axis aligned.
 * @param comm the communicator to build the mesh on
 * @param cells_per_unit number of cells per unit length, in both directions
 */
BackgroundMesh
makeLShapeMesh(const libMesh::Parallel::Communicator & comm, const unsigned int cells_per_unit)
{
  BackgroundMesh background;
  background.mesh = std::make_unique<libMesh::ReplicatedMesh>(comm);
  auto & mesh = *background.mesh;
  mesh.set_mesh_dimension(2);
  mesh.set_spatial_dimension(2);
  mesh.allow_renumbering(false);

  const unsigned int n_cells = 2 * cells_per_unit;
  const Real h = 1.0 / cells_per_unit;

  // A grid node belongs to the L shape unless it lies strictly inside the removed quadrant.
  std::vector<std::vector<libMesh::Node *>> node(
      n_cells + 1, std::vector<libMesh::Node *>(n_cells + 1, nullptr));
  for (const auto i : make_range(n_cells + 1))
    for (const auto j : make_range(n_cells + 1))
      if (i <= cells_per_unit || j <= cells_per_unit)
        node[i][j] = mesh.add_point(Point(i * h, j * h, 0.0));

  for (const auto i : make_range(n_cells))
    for (const auto j : make_range(n_cells))
    {
      if (i >= cells_per_unit && j >= cells_per_unit)
        continue;

      addTri(mesh, node[i][j], node[i + 1][j], node[i + 1][j + 1]);
      addTri(mesh, node[i][j], node[i + 1][j + 1], node[i][j + 1]);
    }

  mesh.prepare_for_use();
  background.boundary_tangent_angles = outgoingBoundaryTangents(mesh);

  return background;
}

/**
 * Background mesh of the equilateral triangle with corners (0, 0), (1, 0) and (0.5, sqrt(3) / 2),
 * built as a triangular lattice so that the mesh carries the three fold symmetry of the domain.
 * @param comm the communicator to build the mesh on
 * @param n_side number of cells along a side, which must be a multiple of three so that the lattice
 * puts a node on the centroid
 */
BackgroundMesh
makeEquilateralTriangleMesh(const libMesh::Parallel::Communicator & comm, const unsigned int n_side)
{
  BackgroundMesh background;
  background.mesh = std::make_unique<libMesh::ReplicatedMesh>(comm);
  auto & mesh = *background.mesh;
  mesh.set_mesh_dimension(2);
  mesh.set_spatial_dimension(2);
  mesh.allow_renumbering(false);

  // The third corner is the origin, which is what lets a lattice point be a weighted sum of these
  // two alone.
  const Point right_corner(1.0, 0.0, 0.0);
  const Point apex(0.5, std::sqrt(3.0) / 2.0, 0.0);

  // node[i][j] sits at the barycentric point (1 - i / n_side - j / n_side, i / n_side, j / n_side).
  std::vector<std::vector<libMesh::Node *>> node(n_side + 1,
                                                 std::vector<libMesh::Node *>(n_side + 1, nullptr));
  for (const auto i : make_range(n_side + 1))
    for (const auto j : make_range(n_side + 1 - i))
      node[i][j] = mesh.add_point(right_corner * (static_cast<Real>(i) / n_side) +
                                  apex * (static_cast<Real>(j) / n_side));

  for (const auto i : make_range(n_side))
    for (const auto j : make_range(n_side - i))
    {
      addTri(mesh, node[i][j], node[i + 1][j], node[i][j + 1]);
      if (i + j + 2 <= n_side)
        addTri(mesh, node[i + 1][j], node[i + 1][j + 1], node[i][j + 1]);
    }

  mesh.prepare_for_use();
  background.boundary_tangent_angles = outgoingBoundaryTangents(mesh);

  return background;
}

/**
 * Locate the singularities of a solved cross field from its phase rather than its magnitude, by
 * accumulating how far arg(z) turns around each element. The turn across an edge is reduced modulo
 * a full turn, so the three edges of an element sum to a whole number of turns, and that number is
 * nonzero only on an element the field winds around, which is an element a zero of z sits in.
 *
 * Summing the winding over every element leaves only the boundary edges, since each interior edge
 * is walked once in each direction, so the total is the winding of z around the domain boundary.
 * That total is four times the index of the cross field, because arg(z) turns four times as fast as
 * the cross it represents.
 *
 * @param mesh the background mesh the field was solved on
 * @param solver a solved cross field solver
 * @return one entry per element the field winds around, empty when the field has no singularity
 */
std::vector<WindingSite>
windingSites(const libMesh::MeshBase & mesh, const CrossFieldSolver & solver)
{
  const auto & nodal_cross_field = solver.nodalCrossField();
  const std::set<dof_id_type> singular_nodes(solver.singularNodes().begin(),
                                             solver.singularNodes().end());

  std::vector<WindingSite> sites;
  for (const auto & elem : mesh.element_ptr_range())
  {
    const auto nodes = counterClockwiseNodes(*elem);

    Real turning = 0.0;
    for (const auto i : make_range(3u))
    {
      const Real from = std::arg(libmesh_map_find(nodal_cross_field, nodes[i]->id()));
      const Real to = std::arg(libmesh_map_find(nodal_cross_field, nodes[(i + 1) % 3]->id()));
      turning += wrapToPi(to - from);
    }

    const int winding = static_cast<int>(std::lround(turning / (2.0 * libMesh::pi)));
    if (winding == 0)
      continue;

    const bool at_singular_node = std::any_of(nodes.begin(),
                                              nodes.end(),
                                              [&singular_nodes](const libMesh::Node * const node)
                                              { return singular_nodes.count(node->id()) == 1; });
    sites.push_back({elem->vertex_average(), winding, at_singular_node});
  }

  return sites;
}

} // namespace

TEST_F(CrossFieldSolverTest, diskBoundaryTangentAngle)
{
  const auto background = makeDiskMesh(_app->comm(), 64, 16);
  CrossFieldSolver solver(*background.mesh, background.boundary_tangent_angles);
  solver.solve();

  for (const auto & [node_id, tangent_angle] : background.boundary_tangent_angles)
  {
    const Point & node_point = background.mesh->point(node_id);
    EXPECT_NEAR(crossAngleDifference(solver.theta(node_point), tangent_angle), 0.0, boundary_tol)
        << "the cross at boundary node " << node_id << " does not lie along the boundary";
  }

  // The exact field on a disk of radius R is z = (r / R)^4 exp(4 i phi), so the cross direction at
  // an interior point is its polar angle modulo a quarter turn. Probed only well away from the
  // center, where z vanishes and carries no direction at all, and off the rays the nodes sit on so
  // that the interpolation inside an element is exercised too.
  for (const Real radius_fraction : {0.85, 0.95})
    for (const auto k : make_range(12u))
    {
      const Real angle = 2.0 * libMesh::pi * k / 12.0 + 0.1;
      const Real radius = radius_fraction * disk_radius;
      const Point probe(radius * std::cos(angle), radius * std::sin(angle), 0.0);

      EXPECT_NEAR(crossAngleDifference(solver.theta(probe), angle), 0.0, interior_tol)
          << "the cross at radius " << radius << " and angle " << angle
          << " does not point along the radius";
    }

  const Point probe(0.6 * disk_radius, 0.5 * disk_radius, 0.0);
  const auto [first_direction, second_direction] = solver.crossFrame(probe);
  const Real angle = solver.theta(probe);
  EXPECT_NEAR(first_direction(0), std::cos(angle), algebraic_tol);
  EXPECT_NEAR(first_direction(1), std::sin(angle), algebraic_tol);
  EXPECT_NEAR(first_direction * second_direction, 0.0, algebraic_tol);
  EXPECT_NEAR(second_direction.norm(), 1.0, algebraic_tol);
}

TEST_F(CrossFieldSolverTest, diskSingularityIndexSum)
{
  const auto background = makeDiskMesh(_app->comm(), 64, 16);
  CrossFieldSolver solver(*background.mesh, background.boundary_tangent_angles);
  solver.solve();

  int total_winding = 0;
  for (const auto & site : windingSites(*background.mesh, solver))
  {
    total_winding += site.winding;

    EXPECT_TRUE(site.at_singular_node)
        << "the cross field winds around the element at (" << site.center(0) << ", "
        << site.center(1) << ") but none of its nodes is reported singular";
    EXPECT_LT(site.center.norm(), 0.25 * disk_radius)
        << "the cross field winds around the element at (" << site.center(0) << ", "
        << site.center(1) << "), away from the center where the only singularity belongs";
  }

  // Each turn of z is a quarter turn of the cross it represents, so the index the disk carries is
  // the winding divided by four. The Poincare-Hopf theorem puts that at +1 for a disk.
  EXPECT_EQ(total_winding, 4);

  // The exact field vanishes only at the center, and its magnitude reaches the 0.1 the solver calls
  // singular at radius 0.1^(1/4) R, which is 0.562 R.
  ASSERT_FALSE(solver.singularNodes().empty());
  Real closest_singular_node = std::numeric_limits<Real>::max();
  for (const auto node_id : solver.singularNodes())
  {
    const Real radius = background.mesh->point(node_id).norm();
    EXPECT_LT(radius, 0.7 * disk_radius) << "node " << node_id << " is reported singular at radius "
                                         << radius << ", where the field is far from vanishing";
    closest_singular_node = std::min(closest_singular_node, radius);
  }
  EXPECT_LT(closest_singular_node, 0.1 * disk_radius);
}

TEST_F(CrossFieldSolverTest, lShapeHasNoInteriorSingularity)
{
  const auto background = makeLShapeMesh(_app->comm(), 8);
  CrossFieldSolver solver(*background.mesh, background.boundary_tangent_angles);
  solver.solve();

  // Every edge of the L shape is axis aligned, so every boundary tangent angle is a multiple of a
  // quarter turn and every boundary value of z is exp(4 i theta_t) = 1, the reentrant corner
  // included. The field is therefore the constant axis aligned cross and has no singularity, which
  // is the same statement as a rectilinear domain needing no irregular vertex in its quad mesh.
  EXPECT_TRUE(solver.singularNodes().empty());

  for (const auto & [node_id, value] : solver.nodalCrossField())
    EXPECT_NEAR(crossAngleDifference(std::arg(value) / 4.0, 0.0), 0.0, boundary_tol)
        << "the cross at node " << node_id << " is not the axis aligned one";

  for (const Point & probe : {Point(0.5, 0.5, 0.0),
                              Point(1.5, 0.5, 0.0),
                              Point(0.5, 1.5, 0.0),
                              Point(0.97, 0.97, 0.0),
                              Point(1.03, 0.9, 0.0)})
    EXPECT_NEAR(crossAngleDifference(solver.theta(probe), 0.0), 0.0, boundary_tol)
        << "the cross at (" << probe(0) << ", " << probe(1) << ") is not the axis aligned one";
}

TEST_F(CrossFieldSolverTest, equilateralTriangleSingularityAtCentroid)
{
  const auto background = makeEquilateralTriangleMesh(_app->comm(), 18);
  CrossFieldSolver solver(*background.mesh, background.boundary_tangent_angles);
  solver.solve();

  // Distance from the centroid to the closest side, which is the length the assertions below
  // measure their radii against.
  const Real inradius = std::sqrt(3.0) / 6.0;
  const Point centroid(0.5, inradius, 0.0);

  int total_winding = 0;
  for (const auto & site : windingSites(*background.mesh, solver))
  {
    total_winding += site.winding;

    EXPECT_TRUE(site.at_singular_node)
        << "the cross field winds around the element at (" << site.center(0) << ", "
        << site.center(1) << ") but none of its nodes is reported singular";
    EXPECT_LT((site.center - centroid).norm(), 0.5 * inradius)
        << "the cross field winds around the element at (" << site.center(0) << ", "
        << site.center(1)
        << "), away from the centroid where the symmetry of the domain puts the "
           "only singularity";
  }

  // Each of the three corners turns the boundary tangent by a third of a turn, which the four fold
  // symmetric z sees as 4 / 3 of a turn, that is, as a third of a turn once whole turns are
  // discarded. The three of them wind z once around the boundary, so the triangle carries a single
  // singularity of index 1/4, the valence five vertex a quad mesh of a triangle needs.
  EXPECT_EQ(total_winding, 1);

  // The domain, its lattice and its boundary data are all invariant under a third of a turn about
  // the centroid, under which z picks up a third of a turn of phase. Only z = 0 satisfies that, so
  // the field vanishes exactly on the node the lattice puts at the centroid.
  ASSERT_FALSE(solver.singularNodes().empty());
  Real closest_singular_node = std::numeric_limits<Real>::max();
  for (const auto node_id : solver.singularNodes())
  {
    const Real distance = (background.mesh->point(node_id) - centroid).norm();
    EXPECT_LT(distance, 0.7 * inradius)
        << "node " << node_id << " is reported singular " << distance
        << " away from the centroid, where the field does not come close to vanishing";
    closest_singular_node = std::min(closest_singular_node, distance);
  }
  EXPECT_NEAR(closest_singular_node, 0.0, algebraic_tol);
}
