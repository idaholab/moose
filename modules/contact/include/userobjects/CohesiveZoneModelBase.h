//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarUserObject.h"
#include "RankTwoTensor.h"

#include <array>

/**
 * Base class for mortar-based cohesive zone models.
 *
 * The object runs on the undisplaced mortar interface, so the mortar segments pair each secondary
 * point with the primary point it is bonded to for the whole simulation. At secondary node j the
 * displacement jump is measured in a local frame that follows the deformed secondary surface:
 *
 *   delta_j = Q_j^T (u_j + dX_j) - Q0_j^T dX_j,
 *
 * where u_j and dX_j are the mortar-weighted primary-minus-secondary displacement and reference
 * position. Q_j = [n, t1, t2] and Q0_j = [N, T1, T2] are the frames that computeInterfaceFrames
 * builds from the deformed and the reference positions of the secondary faces incident to node j.
 * The two frames coincide on the reference mesh, and a rigid motion R of the bonded bodies gives
 * Q_j = R Q0_j and delta_j = 0, whether or not the paired points coincide in the reference
 * configuration.
 */
class CohesiveZoneModelBase : public MortarUserObject
{
public:
  static InputParameters validParams();

  CohesiveZoneModelBase(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void reinit() override;
  virtual void timestepSetup() override;

  /// @return The global traction component \p i interpolated at the mortar segment quadrature
  /// points, per unit reference area
  virtual const ADVariableValue & czmGlobalTraction(unsigned int i) const;

protected:
  /// Computes the mortar integrands that depend only on the current quadrature point
  virtual void computeQpProperties();

  /// Accumulates the nodal mortar integrals at the current quadrature point and test function
  virtual void computeQpIProperties();

  /**
   * Compute the weighted local displacement jump at a secondary node and store it in
   * \p _dof_to_interface_displacement_jump.
   * @param node The secondary node
   * @param orientation_normal The mortar nodal normal at \p node, which orients the incident faces
   * @return The rotation Q from the local interface frame to global coordinates
   */
  ADRankTwoTensor computeLocalDisplacementJump(const Node & node,
                                               const RealVectorValue & orientation_normal);

  /**
   * Build the reference frame Q0 = [N, T1, T2] and the deformed frame Q = [n, t1, t2] of the
   * secondary surface at \p node from the nodes of the secondary faces incident to \p node.
   * N and n are the normalized sums of the reference and deformed face area vectors at the node,
   * and T1, T2 follow from N by CohesiveZoneModelTools::computeReferenceRotation. In 2D, t1 is n
   * rotated by +90 degrees. In 3D, t1 and t2 are T1 and T2 rotated by the in-plane polar rotation
   * of the surface deformation gradient F_s, averaged over the incident faces with their reference
   * areas as weights.
   * @param node The secondary node
   * @param orientation_normal The mortar nodal normal at \p node, which orients the incident faces
   * @return The reference frame and the deformed frame with its displacement derivatives
   */
  std::pair<RankTwoTensor, ADRankTwoTensor>
  computeInterfaceFrames(const Node & node, const RealVectorValue & orientation_normal) const;

  /// Encapsulate the CZM constitutive behavior: store the local traction of \p node in
  /// \p _dof_to_czm_traction
  virtual void computeCZMTraction(const Node * const node) = 0;

  /// Update the damage of \p node
  virtual void computeDamage(const Node * const node) = 0;

  /// Rotate the local traction of \p node to global coordinates with \p rotation
  void computeGlobalTraction(const Node * const node, const ADRankTwoTensor & rotation);

  /// Normalize mortar quantities (remove mortar integral scaling)
  template <class T>
  T normalizeQuantity(const std::unordered_map<const DofObject *, T> & map, const Node * const node)
  {
    return libmesh_map_find(map, node) / libmesh_map_find(_dof_to_normalization, node);
  }

  /// @return The deformed position of \p node, carrying displacement derivatives while they are
  /// recorded for this object's system
  ADPoint deformedPosition(const Node & node) const;

  /// The x displacement variable
  const MooseVariable * const _disp_x_var;

  /// The y displacement variable
  const MooseVariable * const _disp_y_var;

  /// Whether the problem is three-dimensional
  const bool _has_disp_z;

  /// The z displacement variable
  const MooseVariable * const _disp_z_var;

  ///@{ Displacements on the secondary and primary faces at the mortar quadrature points
  const ADVariableValue & _secondary_disp_x;
  const ADVariableValue & _primary_disp_x;
  const ADVariableValue & _secondary_disp_y;
  const ADVariableValue & _primary_disp_y;
  const ADVariableValue * const _secondary_disp_z;
  const ADVariableValue * const _primary_disp_z;
  ///@}

  /// Member for handling change of coordinate systems (xyz, rz, spherical)
  const MooseArray<Real> & _coord;

  /// The time step size
  const Real & _dt;

  /// Whether the nodal mortar integrals are weighted with the dual basis
  const bool _use_dual;

  /// The test functions that weight the nodal mortar integrals and interpolate the traction
  const VariableTestValue * _test = nullptr;

  /// Quadrature point index for the mortar segments
  unsigned int _qp = 0;

  /// Test function index
  unsigned int _i = 0;

  /// The mortar integration weight at the current quadrature point
  Real _qp_factor = 0;

  /// Weighted primary-minus-secondary displacement at the current quadrature point
  ADRealVectorValue _qp_displacement;

  /// Weighted primary-minus-secondary reference position at the current quadrature point
  RealVectorValue _qp_reference_offset;

  /// Map from secondary node to the integral of its test function over the mortar segments
  std::unordered_map<const DofObject *, Real> _dof_to_normalization;

  /// Map from secondary node to the weighted primary-minus-secondary displacement
  std::unordered_map<const DofObject *, ADRealVectorValue> _dof_to_weighted_displacements;

  /// Map from secondary node to the weighted primary-minus-secondary reference position
  std::unordered_map<const DofObject *, RealVectorValue> _dof_to_weighted_reference_offset;

  /// Map from secondary node to the weighted local displacement jump
  std::unordered_map<const DofObject *, ADRealVectorValue> _dof_to_interface_displacement_jump;

  /// Map from secondary node to the local cohesive traction
  std::unordered_map<const DofObject *, ADRealVectorValue> _dof_to_czm_traction;

  /// Map from secondary node to the global cohesive traction
  std::unordered_map<const DofObject *, ADRealVectorValue> _dof_to_global_czm_traction;

  /// The global traction interpolated at the mortar segment quadrature points
  std::array<ADVariableValue, 3> _czm_interpolated_traction;

  /**
   * Tolerance to avoid NaN/Inf in automatic differentiation operations. It is added under square
   * roots of squared jumps, whose derivative is unbounded at zero, and is the threshold for an
   * opening normal jump. 1e-40 is far below the square of any resolvable jump (a jump of 1e-15
   * squares to 1e-30), so it does not change the computed values, while sqrt(1e-40) = 1e-20 keeps
   * the derivatives finite.
   */
  const Real _epsilon_tolerance;

  /// Damage values (pair of current and old) on CZM interface
  std::unordered_map<dof_id_type, std::pair<ADReal, Real>> & _dof_to_damage;
};
