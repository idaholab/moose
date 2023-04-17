//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WeightedGapUserObject.h"

/**
 * Creates dof object to weighted tangential velocities map
 */
class WeightedVelocitiesUserObject : public WeightedGapUserObject
{
public:
  static InputParameters validParams();

  WeightedVelocitiesUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void initialSetup() override;

  /**
   * Get the degree of freedom to weighted velocities information
   */
  const std::unordered_map<const DofObject *, std::array<ADReal, 2>> &
  dofToWeightedVelocities() const;

  /**
   * @return The contact force at quadrature points on the mortar segment
   */
  virtual const ADVariableValue & contactTangentialPressureDirOne() const = 0;
  virtual const ADVariableValue & contactTangentialPressureDirTwo() const = 0;

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

  /// Reference to the EquationSystem object
  SystemBase & _sys;

  /// Reference to the secondary variable
  MooseVariableField<Real> & _secondary_var;

  /// Reference to the primary variable
  MooseVariableField<Real> & _primary_var;

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

  /// Automatic flag to determine whether we are doing three-dimensional work
  bool _3d;
};

inline const std::unordered_map<const DofObject *, std::array<ADReal, 2>> &
WeightedVelocitiesUserObject::dofToWeightedVelocities() const
{
  return _dof_to_weighted_tangential_velocity;
}
