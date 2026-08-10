//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include "libmesh/equation_systems.h"
#include "libmesh/fe_type.h"
#include "libmesh/parallel.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/replicated_mesh.h"

#include <complex>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace libMesh
{
class LinearImplicitSystem;
}

/**
 * Solves for a cross field, the smooth field of 4-fold symmetric directions that orients
 * quad-shaped elements during frontal meshing.
 *
 * The cross field is represented by the complex field z whose argument carries the 4-fold symmetry,
 *
 *   Laplace(z) = 0,   z = exp(4 i theta_t) on the boundary,   theta = arg(z) / 4
 *
 * where theta_t is the boundary tangent angle. Working with z instead of theta makes the problem
 * linear and single valued: theta itself is only defined modulo pi/2, so it can neither be solved
 * for nor interpolated directly. Zeros of z are the field singularities and become the irregular
 * (valence 3 and 5) vertices of the final quad mesh.
 *
 * The solve is a pair of real Laplace problems (one for the real part of z, one for the imaginary
 * part) discretized with linear Lagrange finite elements on the background triangulation. It runs
 * on a private serial communicator so that the field, and therefore the mesh generated from it, is
 * identical on every process and for every process count.
 */
class CrossFieldSolver
{
public:
  /// Values of the complex cross field z at the nodes of the background mesh, keyed by node id.
  using NodalCrossField = std::map<dof_id_type, std::complex<Real>>;

  /**
   * Build a cross field solver on a background triangulation.
   *
   * @param background_mesh Replicated background mesh of TRI3 elements covering the domain. The
   * mesh is copied internally with its node ids preserved, so it need not outlive this object, and
   * its node ids are the ids used by both @p boundary_tangent_angles and every result this class
   * reports.
   * @param boundary_tangent_angles Boundary tangent angle theta_t, in radians, for each boundary
   * node of @p background_mesh, keyed by node id. Every node on the domain boundary must appear;
   * nodes absent from this map are treated as interior.
   */
  CrossFieldSolver(const libMesh::MeshBase & background_mesh,
                   const std::map<dof_id_type, Real> & boundary_tangent_angles);

  /**
   * Assemble and solve the two Laplace problems, then normalize the nodal field and collect the
   * singular nodes. Must be called before any query below.
   */
  void solve();

  /**
   * @return The cross field angle theta at @p point, in radians, obtained by interpolating z over
   * the containing element and taking arg(z) / 4. Since arg() returns a value in (-pi, pi], theta
   * lands in (-pi/4, pi/4], the canonical representative of the cross direction modulo pi/2.
   */
  Real theta(const Point & point) const;

  /**
   * @return The local cross frame at @p point as the pair (u, v), with u = (cos theta, sin theta)
   * and v the in-plane perpendicular of u.
   */
  std::pair<Point, Point> crossFrame(const Point & point) const;

  /**
   * @return The unit-magnitude cross field value z at every node of the background mesh, keyed by
   * node id. A node whose solved magnitude was too small to normalize is assigned z = 1; such a
   * node is always among those reported by singularNodes().
   */
  const NodalCrossField & nodalCrossField() const { return _nodal_cross_field; }

  /**
   * @return Ascending ids of the singular nodes, the nodes where the solved magnitude of z fell
   * below 0.1 before normalization. These become the irregular vertices of the quad mesh.
   */
  const std::vector<dof_id_type> & singularNodes() const { return _singular_nodes; }

private:
  /// libMesh assembly callback; forwards to assembleLaplace() on the solver stashed in @p es.
  static void assembleSystem(libMesh::EquationSystems & es, const std::string & system_name);

  /// Assemble the Laplace operator for both variables and the nodal penalty Dirichlet conditions.
  void assembleLaplace();

  /// Normalize the solved nodal values of z and record the nodes where z vanishes.
  void extractNodalCrossField();

  /// @return The cross field z interpolated over the element containing @p point.
  std::complex<Real> interpolatedCrossField(const Point & point) const;

  /// Serial communicator, so that the solve is reproducible independent of the process count.
  libMesh::Parallel::Communicator _communicator;

  /// Private copy of the background triangulation, sharing the caller's node ids.
  libMesh::ReplicatedMesh _mesh;

  /// Dirichlet value exp(4 i theta_t) of each boundary node, keyed by node id.
  const NodalCrossField _boundary_cross_field;

  /// Linear Lagrange, the discretization of both components of z.
  const libMesh::FEType _fe_type;

  /// Holds the solve; kept past solve() because shape function evaluation needs it.
  std::unique_ptr<libMesh::EquationSystems> _equation_systems;

  /// System holding both components of z; owned by _equation_systems.
  libMesh::LinearImplicitSystem * _system;

  /// Variable number of the real part of z.
  unsigned int _real_variable;

  /// Variable number of the imaginary part of z.
  unsigned int _imaginary_variable;

  /// Locates the element containing a query point.
  std::unique_ptr<libMesh::PointLocatorBase> _point_locator;

  /// Whether solve() has produced the results the queries read.
  bool _solved;

  /// Unit-magnitude nodal values of z, keyed by node id.
  NodalCrossField _nodal_cross_field;

  /// Ascending ids of the nodes where z vanishes.
  std::vector<dof_id_type> _singular_nodes;
};
