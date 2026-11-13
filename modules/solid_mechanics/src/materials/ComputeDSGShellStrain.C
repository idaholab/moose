//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDSGShellStrain.h"

#include "MooseUtils.h"

#include <array>

#include "libmesh/fe_map.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("SolidMechanicsApp", ComputeDSGShellStrain);

using DSGShell::dofs_per_node;
using DSGShell::n_strains;

InputParameters
ComputeDSGShellStrain::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes the generalized strains and the strain-displacement matrix "
                             "of a flat discrete shear gap (DSG) shell element with three, four, "
                             "six or eight nodes.");
  params.addRequiredCoupledVar(
      "displacements",
      "The three displacement variables of the shell nodes, in the order of the global axes X, Y "
      "and Z, for example 'disp_x disp_y disp_z'. Every object of the DSG shell family takes the "
      "same list.");
  params.addRequiredCoupledVar(
      "rotations",
      "The three rotation variables of the shell nodes, the rotations about the global axes X, Y "
      "and Z in that order, for example 'rot_x rot_y rot_z'. The rotation about the normal of an "
      "element is its drilling rotation.");
  params.addParam<RealVectorValue>(
      "reference_first_local_direction",
      RealVectorValue(1, 0, 0),
      "A direction in space whose projection on the plane of each element is the first local "
      "axis of that element, the direction 1 of the resultants N11, M11 and Q1. The default, "
      "the global X axis, gives an element in the XY plane the X axis as its first local axis. "
      "Choose a direction that is not normal to any element of the block, for example the axis "
      "of a cylinder; if the direction is normal to an element, the global axis least aligned "
      "with the element normal is projected instead.");
  params.addParam<bool>(
      "compute_nodal_strains",
      false,
      "Set to true to also evaluate the generalized strains at the nodes of every element, which "
      "the nodal recovery of the resultants by DSGShellNodalResultants needs. Off by default "
      "because it adds one strain evaluation per node to the assembly of every element.");
  return params;
}

ComputeDSGShellStrain::ComputeDSGShellStrain(const InputParameters & parameters)
  : Material(parameters),
    _ndisp(coupledComponents("displacements")),
    _nrot(coupledComponents("rotations")),
    _reference_first_local_direction(getParam<RealVectorValue>("reference_first_local_direction")),
    _strain_displacement_matrix(
        declareProperty<DenseMatrix<Real>>("dsg_shell_strain_displacement_matrix")),
    _generalized_strain(declareProperty<DenseVector<Real>>("dsg_shell_generalized_strain")),
    _rotation_matrix(declareProperty<RankTwoTensor>("dsg_shell_rotation_matrix")),
    _longest_edge(declareProperty<Real>("dsg_shell_longest_edge")),
    _compute_nodal_strains(getParam<bool>("compute_nodal_strains")),
    _nodal_generalized_strain(
        declareProperty<DenseMatrix<Real>>("dsg_shell_nodal_generalized_strain")),
    _n_nodes(0),
    _n_corners(0)
{
  if (_ndisp != 3 || _nrot != 3)
    paramError("displacements",
               "'displacements' must list three variables and 'rotations' "
               "must list three variables; the DSG shell has six degrees of freedom per node.");

  if (MooseUtils::absoluteFuzzyEqual(_reference_first_local_direction.norm(), 0.0))
    paramError("reference_first_local_direction", "The reference direction must not be zero.");

  // The degrees of freedom of the current element are read variable by variable, in the order
  // used throughout the element: the three displacements, then the three rotations.
  for (unsigned int i = 0; i < _ndisp; ++i)
    _dof_values.push_back(&coupledDofValues("displacements", i));
  for (unsigned int i = 0; i < _nrot; ++i)
    _dof_values.push_back(&coupledDofValues("rotations", i));
}

void
ComputeDSGShellStrain::computeProperties()
{
  // The family covers the linear and quadratic triangles and quadrilaterals, in their plain
  // form and in the shell form that mesh generators write for surfaces in space.
  const ElemType type = _current_elem->type();
  const bool triangle =
      (type == libMesh::TRI3 || type == libMesh::TRISHELL3 || type == libMesh::TRI6);
  const bool quadrilateral = (type == libMesh::QUAD4 || type == libMesh::QUADSHELL4 ||
                              type == libMesh::QUAD8 || type == libMesh::QUADSHELL8);
  if (!triangle && !quadrilateral)
    mooseError("Element ",
               _current_elem->id(),
               " is of type ",
               libMesh::Utility::enum_to_string(type),
               "; the DSG shell family covers TRI3, QUAD4, TRI6 and QUAD8 elements and their "
               "TRISHELL3, QUADSHELL4 and QUADSHELL8 forms.");

  _n_nodes = _current_elem->n_nodes();
  _n_corners = triangle ? 3 : 4;

  for (const auto variable : index_range(_dof_values))
    if (_dof_values[variable]->size() != _n_nodes)
      mooseError("Variable ",
                 variable < _ndisp ? getVar("displacements", variable)->name()
                                   : getVar("rotations", variable - _ndisp)->name(),
                 " has ",
                 _dof_values[variable]->size(),
                 " degrees of freedom on element ",
                 _current_elem->id(),
                 ", which has ",
                 _n_nodes,
                 " nodes. The displacement and rotation variables must be Lagrange variables of "
                 "the element's order: FIRST on TRI3 and QUAD4, SECOND on TRI6 and QUAD8.");

  computeElementFrame();
  computeShearGapMatrices();

  // The nodal degrees of freedom in the global frame, node by node: ux uy uz rx ry rz.
  const unsigned int n_dofs = dofs_per_node * _n_nodes;
  DenseVector<Real> global_dofs(n_dofs);
  for (unsigned int node = 0; node < _n_nodes; ++node)
    for (unsigned int component = 0; component < dofs_per_node; ++component)
      global_dofs(dofs_per_node * node + component) = (*_dof_values[component])[node];

  // The transformation from global to local degrees of freedom rotates every displacement and
  // rotation triplet with the rotation matrix of the element frame.
  DenseMatrix<Real> global_to_local(n_dofs, n_dofs);
  for (unsigned int triplet = 0; triplet < 2 * _n_nodes; ++triplet)
    for (unsigned int row = 0; row < 3; ++row)
      for (unsigned int column = 0; column < 3; ++column)
        global_to_local(3 * triplet + row, 3 * triplet + column) = _rotation(row, column);

  // The longest edge is an element constant, used by the thin-limit factor of the three-node
  // element
  Real longest_edge = 0.0;
  for (unsigned int corner = 0; corner < _n_corners; ++corner)
  {
    const unsigned int next_corner = (corner + 1) % _n_corners;
    const Real edge = (_local_coordinates[next_corner] - _local_coordinates[corner]).norm();
    longest_edge = std::max(longest_edge, edge);
  }

  DenseMatrix<Real> B_local(n_strains, n_dofs);
  DenseMatrix<Real> B_global(n_strains, n_dofs);
  DenseVector<Real> strain(n_strains);

  // The strains at the nodes, for the nodal recovery of the resultants: the element's own
  // interpolation evaluated at the parent coordinates of each node, one row per node
  DenseMatrix<Real> nodal_strain;
  if (_compute_nodal_strains)
  {
    nodal_strain.resize(_n_nodes, n_strains);
    for (unsigned int node = 0; node < _n_nodes; ++node)
    {
      const Point & parent_node = _parent_coordinates[node];
      localStrainDisplacementMatrix(parent_node(0), parent_node(1), B_local);
      B_global = B_local;
      B_global.right_multiply(global_to_local);
      B_global.vector_mult(strain, global_dofs);
      for (unsigned int component = 0; component < n_strains; ++component)
        nodal_strain(node, component) = strain(component);
    }
  }

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // The quadrature point is mapped back to the parent element so that the same matrices are
    // built whether the material is evaluated inside the element or on one of its edges, as it
    // is when an integrated boundary condition or a side postprocessor asks for it there.
    const Point parent_point = libMesh::FEMap::inverse_map(2, _current_elem, _q_point[_qp]);

    localStrainDisplacementMatrix(parent_point(0), parent_point(1), B_local);
    B_global = B_local;
    B_global.right_multiply(global_to_local);
    B_global.vector_mult(strain, global_dofs);

    _strain_displacement_matrix[_qp] = B_global;
    _generalized_strain[_qp] = strain;
    _rotation_matrix[_qp] = _rotation;
    _longest_edge[_qp] = longest_edge;
    // The nodal strains are an element quantity, stored once, at the first quadrature point
    if (_qp == 0)
      _nodal_generalized_strain[_qp] = nodal_strain;
    else
      _nodal_generalized_strain[_qp].resize(0, 0);
  }
}

void
ComputeDSGShellStrain::computeElementFrame()
{
  std::vector<Point> nodes(_n_nodes);
  for (unsigned int node = 0; node < _n_nodes; ++node)
    nodes[node] = _current_elem->point(node);

  // The normal comes from the corner nodes: the two diagonals of a quadrilateral, the two edges
  // leaving the first node of a triangle.
  Point first_direction;
  Point second_direction;
  if (_n_corners == 4)
  {
    first_direction = nodes[2] - nodes[0];
    second_direction = nodes[3] - nodes[1];
  }
  else
  {
    first_direction = nodes[1] - nodes[0];
    second_direction = nodes[2] - nodes[0];
  }
  Point normal = first_direction.cross(second_direction);
  normal /= normal.norm();

  // The first in-plane axis is the projection of the reference direction on the element plane.
  // When the reference direction is parallel to the normal, within the 0.05 degrees the
  // four-node shell of the module uses, the global axis least aligned with the normal is
  // projected instead; that axis always has a projection.
  Point reference = _reference_first_local_direction;
  Point first_axis = reference - (reference * normal) * normal;
  if (first_axis.norm() < reference.norm() * std::sin(0.05 * libMesh::pi / 180.0))
  {
    unsigned int least_aligned_axis = 0;
    for (unsigned int axis = 1; axis < 3; ++axis)
      if (std::abs(normal(axis)) < std::abs(normal(least_aligned_axis)))
        least_aligned_axis = axis;
    reference = Point(0, 0, 0);
    reference(least_aligned_axis) = 1.0;
    first_axis = reference - (reference * normal) * normal;
  }
  first_axis /= first_axis.norm();
  const Point second_axis = normal.cross(first_axis);

  _rotation = RankTwoTensor::initializeFromRows(first_axis, second_axis, normal);

  // The in-plane coordinates are measured from the corner centroid of a quadrilateral and from
  // the first node of a triangle. The choice of origin does not change any strain.
  Point origin = nodes[0];
  if (_n_corners == 4)
    origin = 0.25 * (nodes[0] + nodes[1] + nodes[2] + nodes[3]);

  _local_coordinates.resize(_n_nodes);
  for (unsigned int node = 0; node < _n_nodes; ++node)
  {
    const Point offset = nodes[node] - origin;
    _local_coordinates[node] = Point(offset * first_axis, offset * second_axis, 0.0);
  }

  // The parent coordinates follow the libMesh node order: corners first, then the midside node
  // of every edge in the order of the edges.
  if (_n_corners == 3)
    _parent_coordinates = {Point(0.0, 0.0),
                           Point(1.0, 0.0),
                           Point(0.0, 1.0),
                           Point(0.5, 0.0),
                           Point(0.5, 0.5),
                           Point(0.0, 0.5)};
  else
    _parent_coordinates = {Point(-1.0, -1.0),
                           Point(1.0, -1.0),
                           Point(1.0, 1.0),
                           Point(-1.0, 1.0),
                           Point(0.0, -1.0),
                           Point(1.0, 0.0),
                           Point(0.0, 1.0),
                           Point(-1.0, 0.0)};
  _parent_coordinates.resize(_n_nodes);
}

void
ComputeDSGShellStrain::shapeFunctions(const Real xi,
                                      const Real eta,
                                      std::vector<Real> & N,
                                      std::vector<Real> & dN_dxi,
                                      std::vector<Real> & dN_deta) const
{
  N.assign(_n_nodes, 0.0);
  dN_dxi.assign(_n_nodes, 0.0);
  dN_deta.assign(_n_nodes, 0.0);

  // Signs of the parent coordinates at the corners of a quadrilateral
  const std::array<Real, 4> corner_xi = {-1.0, 1.0, 1.0, -1.0};
  const std::array<Real, 4> corner_eta = {-1.0, -1.0, 1.0, 1.0};

  if (_n_nodes == 3)
  {
    // Linear triangle
    N[0] = 1.0 - xi - eta;
    N[1] = xi;
    N[2] = eta;
    dN_dxi[0] = -1.0;
    dN_dxi[1] = 1.0;
    dN_deta[0] = -1.0;
    dN_deta[2] = 1.0;
  }
  else if (_n_nodes == 6)
  {
    // Quadratic triangle in area coordinates
    const Real L1 = 1.0 - xi - eta;
    const Real L2 = xi;
    const Real L3 = eta;
    N[0] = L1 * (2.0 * L1 - 1.0);
    N[1] = L2 * (2.0 * L2 - 1.0);
    N[2] = L3 * (2.0 * L3 - 1.0);
    N[3] = 4.0 * L1 * L2;
    N[4] = 4.0 * L2 * L3;
    N[5] = 4.0 * L3 * L1;
    dN_dxi[0] = -(4.0 * L1 - 1.0);
    dN_dxi[1] = 4.0 * L2 - 1.0;
    dN_dxi[2] = 0.0;
    dN_dxi[3] = 4.0 * (L1 - L2);
    dN_dxi[4] = 4.0 * L3;
    dN_dxi[5] = -4.0 * L3;
    dN_deta[0] = -(4.0 * L1 - 1.0);
    dN_deta[1] = 0.0;
    dN_deta[2] = 4.0 * L3 - 1.0;
    dN_deta[3] = -4.0 * L2;
    dN_deta[4] = 4.0 * L2;
    dN_deta[5] = 4.0 * (L1 - L3);
  }
  else if (_n_nodes == 4)
  {
    // Bilinear quadrilateral
    for (unsigned int corner = 0; corner < 4; ++corner)
    {
      N[corner] = 0.25 * (1.0 + corner_xi[corner] * xi) * (1.0 + corner_eta[corner] * eta);
      dN_dxi[corner] = 0.25 * corner_xi[corner] * (1.0 + corner_eta[corner] * eta);
      dN_deta[corner] = 0.25 * corner_eta[corner] * (1.0 + corner_xi[corner] * xi);
    }
  }
  else
  {
    // Serendipity quadrilateral: the corners, then the midside nodes
    for (unsigned int corner = 0; corner < 4; ++corner)
    {
      const Real sx = corner_xi[corner] * xi;
      const Real se = corner_eta[corner] * eta;
      N[corner] = 0.25 * (1.0 + sx) * (1.0 + se) * (sx + se - 1.0);
      dN_dxi[corner] = 0.25 * corner_xi[corner] * (1.0 + se) * (2.0 * sx + se);
      dN_deta[corner] = 0.25 * corner_eta[corner] * (1.0 + sx) * (sx + 2.0 * se);
    }
    N[4] = 0.5 * (1.0 - xi * xi) * (1.0 - eta);
    dN_dxi[4] = -xi * (1.0 - eta);
    dN_deta[4] = -0.5 * (1.0 - xi * xi);
    N[5] = 0.5 * (1.0 + xi) * (1.0 - eta * eta);
    dN_dxi[5] = 0.5 * (1.0 - eta * eta);
    dN_deta[5] = -eta * (1.0 + xi);
    N[6] = 0.5 * (1.0 - xi * xi) * (1.0 + eta);
    dN_dxi[6] = -xi * (1.0 + eta);
    dN_deta[6] = 0.5 * (1.0 - xi * xi);
    N[7] = 0.5 * (1.0 - xi) * (1.0 - eta * eta);
    dN_dxi[7] = -0.5 * (1.0 - eta * eta);
    dN_deta[7] = -eta * (1.0 - xi);
  }
}

Real
ComputeDSGShellStrain::rotationLineIntegral(const std::vector<Real> & beta_1,
                                            const std::vector<Real> & beta_2,
                                            const unsigned int node_a,
                                            const unsigned int node_b) const
{
  const Point & a = _local_coordinates[node_a];
  const Point & b = _local_coordinates[node_b];

  // On the linear elements the rotation varies linearly along the straight edge, so the
  // trapezoidal rule is exact.
  if (_n_nodes == 3 || _n_nodes == 4)
    return 0.5 * ((beta_1[node_a] + beta_1[node_b]) * (b(0) - a(0)) +
                  (beta_2[node_a] + beta_2[node_b]) * (b(1) - a(1)));

  // On the quadratic elements the line is straight in the parent element and the integrand is
  // cubic, which the two-point Gauss rule integrates exactly.
  const Point & parent_a = _parent_coordinates[node_a];
  const Point & parent_b = _parent_coordinates[node_b];
  const std::array<Real, 2> gauss_points = {-1.0 / std::sqrt(3.0), 1.0 / std::sqrt(3.0)};

  std::vector<Real> & N = _shape_functions;
  std::vector<Real> & dN_dxi = _shape_derivatives_xi;
  std::vector<Real> & dN_deta = _shape_derivatives_eta;
  Real integral = 0.0;
  for (const Real gauss_point : gauss_points)
  {
    const Real s = 0.5 + 0.5 * gauss_point;
    const Real xi = parent_a(0) + (parent_b(0) - parent_a(0)) * s;
    const Real eta = parent_a(1) + (parent_b(1) - parent_a(1)) * s;
    shapeFunctions(xi, eta, N, dN_dxi, dN_deta);

    // The tangent of the line in local coordinates, and the rotation interpolated on the line
    Real dx_ds = 0.0;
    Real dy_ds = 0.0;
    Real beta_1_on_line = 0.0;
    Real beta_2_on_line = 0.0;
    for (unsigned int node = 0; node < _n_nodes; ++node)
    {
      const Real dN_ds =
          (parent_b(0) - parent_a(0)) * dN_dxi[node] + (parent_b(1) - parent_a(1)) * dN_deta[node];
      dx_ds += dN_ds * _local_coordinates[node](0);
      dy_ds += dN_ds * _local_coordinates[node](1);
      beta_1_on_line += N[node] * beta_1[node];
      beta_2_on_line += N[node] * beta_2[node];
    }
    integral += 0.5 * (beta_1_on_line * dx_ds + beta_2_on_line * dy_ds);
  }
  return integral;
}

void
ComputeDSGShellStrain::computeShearGapMatrices()
{
  // The shear gap lines of every member of the family, as pairs of (gap node, reference node)
  // in the libMesh node numbering of the parent element:
  //
  //   triangles (xi, eta):                     quadrilaterals (xi, eta):
  //   2                                        3 --- 6 --- 2
  //   | \        0 (0,0)  1 (1,0)  2 (0,1)      |           |     0 (-1,-1)  1 (1,-1)
  //   5   4      3, 4, 5 on the edges           7           5     2 (1,1)    3 (-1,1)
  //   |     \    0-1, 1-2, 2-0                  |           |     4, 5, 6, 7 on the edges
  //   0 - 3 - 1                                 0 --- 4 --- 1     0-1, 1-2, 2-3, 3-0
  //
  // The gap of the reference node is zero by definition, so only the gap node gets an entry.
  // The three-node element measures one gap field from its first node in both parent
  // directions; the four-node element measures the gap along its edges, in the direction of
  // each edge (the DSG4 of Bletzinger, Bischoff and Ramm, 2000); the quadratic elements measure
  // it along the lines of nodes of the parent element, corner to midside and midside to corner.
  std::vector<std::pair<unsigned int, unsigned int>> lines_xi;
  std::vector<std::pair<unsigned int, unsigned int>> lines_eta;
  if (_n_nodes == 3)
  {
    lines_xi = {{1, 0}, {2, 0}};
    lines_eta = {{1, 0}, {2, 0}};
  }
  else if (_n_nodes == 4)
  {
    lines_xi = {{1, 0}, {2, 3}};
    lines_eta = {{2, 1}, {3, 0}};
  }
  else if (_n_nodes == 6)
  {
    lines_xi = {{3, 0}, {1, 0}, {4, 5}};
    lines_eta = {{5, 0}, {2, 0}, {4, 3}};
  }
  else
  {
    lines_xi = {{4, 0}, {1, 0}, {5, 7}, {6, 3}, {2, 3}};
    lines_eta = {{7, 0}, {3, 0}, {6, 4}, {5, 1}, {2, 1}};
  }

  const unsigned int n_dofs = dofs_per_node * _n_nodes;
  _gap_matrix_xi.resize(_n_nodes, n_dofs);
  _gap_matrix_eta.resize(_n_nodes, n_dofs);

  // Each local degree of freedom is given a unit value in turn. A transverse displacement moves
  // the gap directly. The rotations enter through the quantities the gap subtracts,
  // beta_1 = -theta_2 and beta_2 = theta_1, with theta_1 and theta_2 the rotations about the
  // first and second local axes: the normal moves by z theta_2 along the first axis and by
  // -z theta_1 along the second, so the transverse shear strains are
  // w,1 + theta_2 = w,1 - beta_1 and w,2 - theta_1 = w,2 - beta_2.
  std::vector<Real> w(_n_nodes);
  std::vector<Real> beta_1(_n_nodes);
  std::vector<Real> beta_2(_n_nodes);
  for (unsigned int dof = 0; dof < n_dofs; ++dof)
  {
    const unsigned int node = dof / dofs_per_node;
    const unsigned int component = dof % dofs_per_node;

    std::fill(w.begin(), w.end(), 0.0);
    std::fill(beta_1.begin(), beta_1.end(), 0.0);
    std::fill(beta_2.begin(), beta_2.end(), 0.0);
    if (component == 2)
      w[node] = 1.0;
    else if (component == 3)
      beta_2[node] = 1.0;
    else if (component == 4)
      beta_1[node] = -1.0;
    else
      continue;

    for (const auto & line : lines_xi)
    {
      const unsigned int gap_node = line.first;
      const unsigned int reference_node = line.second;
      _gap_matrix_xi(gap_node, dof) =
          (w[gap_node] - w[reference_node]) -
          rotationLineIntegral(beta_1, beta_2, reference_node, gap_node);
    }
    for (const auto & line : lines_eta)
    {
      const unsigned int gap_node = line.first;
      const unsigned int reference_node = line.second;
      _gap_matrix_eta(gap_node, dof) =
          (w[gap_node] - w[reference_node]) -
          rotationLineIntegral(beta_1, beta_2, reference_node, gap_node);
    }
  }
}

void
ComputeDSGShellStrain::localStrainDisplacementMatrix(const Real xi,
                                                     const Real eta,
                                                     DenseMatrix<Real> & B_local) const
{
  std::vector<Real> & N = _shape_functions;
  std::vector<Real> & dN_dxi = _shape_derivatives_xi;
  std::vector<Real> & dN_deta = _shape_derivatives_eta;
  shapeFunctions(xi, eta, N, dN_dxi, dN_deta);

  // The Jacobian of the parent-to-local map and its inverse
  Real J[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
  for (unsigned int node = 0; node < _n_nodes; ++node)
  {
    J[0][0] += dN_dxi[node] * _local_coordinates[node](0);
    J[0][1] += dN_dxi[node] * _local_coordinates[node](1);
    J[1][0] += dN_deta[node] * _local_coordinates[node](0);
    J[1][1] += dN_deta[node] * _local_coordinates[node](1);
  }
  const Real determinant = J[0][0] * J[1][1] - J[0][1] * J[1][0];
  if (determinant <= 0.0)
    mooseError("Element ",
               _current_elem->id(),
               " has a non-positive Jacobian in its own plane; check the node order and the "
               "shape of the element.");
  const Real inverse[2][2] = {{J[1][1] / determinant, -J[0][1] / determinant},
                              {-J[1][0] / determinant, J[0][0] / determinant}};

  std::vector<Real> dN_dx(_n_nodes);
  std::vector<Real> dN_dy(_n_nodes);
  for (unsigned int node = 0; node < _n_nodes; ++node)
  {
    dN_dx[node] = inverse[0][0] * dN_dxi[node] + inverse[0][1] * dN_deta[node];
    dN_dy[node] = inverse[1][0] * dN_dxi[node] + inverse[1][1] * dN_deta[node];
  }

  const unsigned int n_dofs = dofs_per_node * _n_nodes;
  B_local.resize(n_strains, n_dofs);

  for (unsigned int node = 0; node < _n_nodes; ++node)
  {
    const unsigned int ux = dofs_per_node * node;
    const unsigned int uy = ux + 1;
    const unsigned int rx = ux + 3;
    const unsigned int ry = ux + 4;
    const unsigned int rz = ux + 5;

    // Membrane strains: e11, e22 and the engineering shear 2 e12
    B_local(0, ux) = dN_dx[node];
    B_local(1, uy) = dN_dy[node];
    B_local(2, ux) = dN_dy[node];
    B_local(2, uy) = dN_dx[node];

    // Curvatures, with the sign of the module's four-node shell: a positive curvature gives a
    // positive moment, the integral of the stress times the distance from the mid-surface, so
    // that the face the normal points to is in tension. The normal moves by z theta_2 along
    // the first axis and by -z theta_1 along the second, hence the signs on the rotations.
    B_local(3, ry) = dN_dx[node];
    B_local(4, rx) = -dN_dy[node];
    B_local(5, ry) = dN_dy[node];
    B_local(5, rx) = -dN_dx[node];

    // Drilling strain: the drilling rotation minus the rotation of the in-plane displacement
    B_local(8, ux) = 0.5 * dN_dy[node];
    B_local(8, uy) = -0.5 * dN_dx[node];
    B_local(8, rz) = N[node];
  }

  // Transverse shear strains from the two shear gap fields: the fields are interpolated with
  // the shape functions, differentiated along the parent directions and mapped to the local
  // axes with the inverse Jacobian.
  for (unsigned int dof = 0; dof < n_dofs; ++dof)
  {
    Real gap_derivative_xi = 0.0;
    Real gap_derivative_eta = 0.0;
    for (unsigned int node = 0; node < _n_nodes; ++node)
    {
      gap_derivative_xi += dN_dxi[node] * _gap_matrix_xi(node, dof);
      gap_derivative_eta += dN_deta[node] * _gap_matrix_eta(node, dof);
    }
    B_local(6, dof) = inverse[0][0] * gap_derivative_xi + inverse[0][1] * gap_derivative_eta;
    B_local(7, dof) = inverse[1][0] * gap_derivative_xi + inverse[1][1] * gap_derivative_eta;
  }
}
