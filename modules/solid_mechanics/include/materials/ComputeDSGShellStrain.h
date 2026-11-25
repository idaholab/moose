//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DenseMatrix.h"
#include "libmesh/dense_vector.h"
#include "RankTwoTensor.h"

/**
 * ComputeDSGShellStrain computes the generalized strains of a flat shell element
 * of the discrete shear gap (DSG) family: the three-node and six-node triangles
 * and the four-node and eight-node quadrilaterals.
 *
 * The element has six degrees of freedom per node (three displacements and
 * three rotations). The membrane strains come from the in-plane displacements,
 * the curvatures from the in-plane rotations, and the transverse shear strains
 * from the discrete shear gaps of Bletzinger, Bischoff and Ramm (2000), which
 * remove shear locking without a reduced integration. The drilling rotation is
 * tied to the in-plane rotation of the displacement field by a drilling strain.
 *
 * The shear gap matrices of the element are assembled once per element. The
 * strain-displacement matrix is then built at every quadrature point and
 * stored there, together with the generalized strains, the rotation matrix of
 * the element frame and the longest edge.
 *
 * Every object of the family reads the strain-displacement matrix with the
 * conventions of the DSGShell namespace below: nine generalized strains in
 * rows, and the degrees of freedom in columns ordered node by node, each node
 * with its three displacements followed by its three rotations about the
 * global axes.
 *
 * The strain-displacement matrix is stored as a material property at every quadrature
 * point, 9 rows by six columns per node, so that the kernels assemble the residual and the
 * exact Jacobian from it without any further element computation. The element is linear,
 * so the matrix is a function of the geometry alone and the storage is the whole cost.
 */

namespace DSGShell
{
/// Generalized strain components: 3 membrane, 3 bending, 2 transverse shear, 1 drilling
constexpr unsigned int n_strains = 9;

/// Degrees of freedom per node: three displacements followed by three rotations
constexpr unsigned int dofs_per_node = 6;
}

class ComputeDSGShellStrain : public Material
{
public:
  static InputParameters validParams();

  ComputeDSGShellStrain(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;

  /**
   * Builds the element frame from the node coordinates: the normal from the
   * corner nodes, the first in-plane axis from the projected reference
   * direction, and the in-plane coordinates of every node measured from the
   * element origin (the first node of a triangle, the corner centroid of a
   * quadrilateral).
   */
  void computeElementFrame();

  /**
   * Evaluates the shape functions and their derivatives with respect to the
   * parent coordinates at one parent point, for the element's node count.
   */
  void shapeFunctions(const Real xi,
                      const Real eta,
                      std::vector<Real> & N,
                      std::vector<Real> & dN_dxi,
                      std::vector<Real> & dN_deta) const;

  /**
   * Returns the integral along the straight parent line from node a to node b
   * of the quantity the shear gap subtracts, beta_1 = -theta_2 and
   * beta_2 = theta_1 given by their nodal values. The integral is the
   * trapezoidal rule on the linear elements and a two-point Gauss rule along
   * the parent line on the quadratic ones, which is exact for their cubic
   * integrand.
   */
  Real rotationLineIntegral(const std::vector<Real> & beta_1,
                            const std::vector<Real> & beta_2,
                            const unsigned int node_a,
                            const unsigned int node_b) const;

  /**
   * Builds the two shear gap matrices. Row k of a matrix is the shear gap of
   * node k along one parent direction, written as a linear form of the local
   * degrees of freedom of the element.
   */
  void computeShearGapMatrices();

  /**
   * Builds the strain-displacement matrix at one parent point in the local
   * frame: the membrane, bending, transverse shear and drilling rows.
   */
  void
  localStrainDisplacementMatrix(const Real xi, const Real eta, DenseMatrix<Real> & B_local) const;

  /// Number of coupled displacement variables (must be three)
  const unsigned int _ndisp;

  /// Number of coupled rotation variables (must be three)
  const unsigned int _nrot;

  /// Degrees of freedom of the current element for each of the six variables, node by node
  std::vector<const VariableValue *> _dof_values;

  /// Direction projected on the element plane to define the first local axis
  const RealVectorValue _reference_first_local_direction;

  /// Strain-displacement matrix (9 rows by six times the node count), in global degrees of freedom
  MaterialProperty<DenseMatrix<Real>> & _strain_displacement_matrix;

  /// Generalized strains: two membrane strains and the engineering membrane shear, three curvatures, two transverse shear strains and the drilling strain
  MaterialProperty<DenseVector<Real>> & _generalized_strain;

  /// Rotation matrix of the element frame; its rows are the first in-plane axis, the second in-plane axis and the normal
  MaterialProperty<RankTwoTensor> & _rotation_matrix;

  /// Longest edge of the element, used by the thin-limit factor of the three-node element
  MaterialProperty<Real> & _longest_edge;

  /// Shape functions and their parent derivatives at the point last evaluated (one allocation)
  mutable std::vector<Real> _shape_functions;
  mutable std::vector<Real> _shape_derivatives_xi;
  mutable std::vector<Real> _shape_derivatives_eta;

  /// Whether the generalized strains are also evaluated at the nodes of the element
  const bool _compute_nodal_strains;

  /// Generalized strains at the nodes, one row per node, when requested, stored at the first
  /// quadrature point of the element only; empty otherwise
  MaterialProperty<DenseMatrix<Real>> & _nodal_generalized_strain;

  /// Number of nodes of the current element
  unsigned int _n_nodes;

  /// Number of corner nodes of the current element (three or four)
  unsigned int _n_corners;

  /// Parent coordinates of the nodes of the current element
  std::vector<Point> _parent_coordinates;

  /// In-plane coordinates of the nodes of the current element in the element frame
  std::vector<Point> _local_coordinates;

  /// Rotation matrix of the current element (rows: first axis, second axis, normal)
  RankTwoTensor _rotation;

  /// Shear gap matrix along the first parent direction (node count by local degrees of freedom)
  DenseMatrix<Real> _gap_matrix_xi;

  /// Shear gap matrix along the second parent direction (node count by local degrees of freedom)
  DenseMatrix<Real> _gap_matrix_eta;
};
