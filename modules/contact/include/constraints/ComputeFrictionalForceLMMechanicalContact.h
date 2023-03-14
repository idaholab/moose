//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeWeightedGapLMMechanicalContact.h"
#include "Function.h"

class WeightedVelocitiesUserObject;

/**
 * Computes frictional constraints (and normal contact constraints by calling its parent object)
 */
class ComputeFrictionalForceLMMechanicalContact : public ComputeWeightedGapLMMechanicalContact
{
public:
  static InputParameters validParams();

  ComputeFrictionalForceLMMechanicalContact(const InputParameters & parameters);
  void residualSetup() override;
  void post() override;

  /**
   * Copy of the post routine but that skips assembling inactive nodes
   */
  void
  incorrectEdgeDroppingPost(const std::unordered_set<const Node *> & inactive_lm_nodes) override;

protected:
  /**
   * Computes properties that are functions only of the current quadrature point (\p _qp), e.g.
   * indepedent of shape functions
   */
  virtual void computeQpProperties() override;

  /**
   * Computes properties that are functions both of \p _qp and \p _i, for example the weighted gap
   */
  virtual void computeQpIProperties() override;

  /**
   * Method called from \p post(). Used to enforce node-associated constraints. E.g. for the base \p
   * ComputeFrictionalForceLMMechanicalContact we enforce PDASS frictional constraints. This is also
   * where we actually feed the node-based constraint information into the system residual and
   * Jacobian
   */
  virtual void enforceConstraintOnDof(const DofObject * const dof) override;

  /**
   * Method called from \p post(). Used to enforce node-associated constraints. E.g. for the base \p
   * ComputeFrictionalForceLMMechanicalContact we enforce PDASS frictional constraints. This is also
   * where we actually feed the node-based constraint information into the system residual and
   * Jacobian. Method used to enforce frictional constraints in three-dimensional simulations.
   */
  virtual void enforceConstraintOnDof3d(const DofObject * const dof);

  /**
   * Compute coefficient of friction. Allows the use of contact pressure and relative velocity
   * dependent friction.
   */
  ADReal computeFrictionValue(const ADReal & contact_pressure,
                              const ADReal & tangential_vel,
                              const ADReal & tangential_vel_dir);

  /// A map from node to two weighted tangential velocities
  std::unordered_map<const DofObject *, std::array<ADReal, 2>> _dof_to_weighted_tangential_velocity;

  /// A map from node to two interpolated, physical tangential velocities
  std::unordered_map<const DofObject *, std::array<ADReal, 2>> _dof_to_real_tangential_velocity;

  /// An array of two pointers to avoid copies
  std::array<const ADReal *, 2> _tangential_vel_ptr = {{nullptr, nullptr}};

  /// The value of the tangential velocity values at the current quadrature point
  std::array<ADReal, 2> _qp_tangential_velocity;

  /// The value of the "real" tangential velocity values at the current quadrature point
  std::array<ADReal, 2> _qp_real_tangential_velocity;

  /// The value of the tangential velocity vectors at the current node
  ADRealVectorValue _qp_tangential_velocity_nodal;

  /// The value of the real tangential velocity vectors at the current node
  ADRealVectorValue _qp_real_tangential_velocity_nodal;

  /// The weighted gap user object
  const WeightedVelocitiesUserObject & _weighted_velocities_uo;

  /// Numerical factor used in the tangential constraints for convergence purposes
  const Real _c_t;

  /// Frictional Lagrange's multiplier variable pointers
  std::vector<MooseVariable *> _friction_vars;

  /// x-velocity on the secondary face
  const ADVariableValue & _secondary_x_dot;

  /// x-velocity on the primary face
  const ADVariableValue & _primary_x_dot;

  /// y-velocity on the secondary face
  const ADVariableValue & _secondary_y_dot;

  /// y-velocity on the primary face
  const ADVariableValue & _primary_y_dot;

  /// z-velocity on the secondary face
  const ADVariableValue * const _secondary_z_dot;

  /// z-velocity on the primary face
  const ADVariableValue * const _primary_z_dot;

  /// Small contact pressure value to trigger computation of frictional forces
  const Real _epsilon;

  /// Friction coefficient
  const Real _mu;

  /// input function
  const Function * const _function_friction;

  /// Boolean to determine whether the friction coefficient is taken from a function
  const bool _has_friction_function;

  /// Automatic flag to determine whether we are doing three-dimensional work
  bool _3d;
};
