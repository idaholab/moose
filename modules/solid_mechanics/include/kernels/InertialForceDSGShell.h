//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeKernel.h"
#include "RankTwoTensor.h"

/**
 * InertialForceDSGShell computes the inertial force of a DSG shell element for
 * one of its six degrees of freedom, with mass-proportional Rayleigh damping
 * and the damping term of the HHT time integration scheme.
 *
 * The translational mass is the consistent mass of the shell mid-surface, the
 * density times the thickness integrated with the shape functions. The rotary
 * inertia acts on the two rotations in the plane of the element; the drilling
 * rotation is given no inertia and is held by the drilling stiffness alone. The velocities
 * and accelerations come from the time integrator, which must provide the
 * second time derivative of the solution (the NewmarkBeta time integrator).
 */
class InertialForceDSGShell : public TimeKernel
{
public:
  static InputParameters validParams();

  InertialForceDSGShell(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Returns the acceleration of one degree of freedom with its damping terms added
  Real dampedAcceleration(const unsigned int component) const;

  /// Returns the derivative of the damped acceleration with respect to the degree of freedom
  Real dampedAccelerationDerivative(const unsigned int component) const;

  /// Returns the entry of the in-plane projector between two global rotation axes
  Real inPlaneProjector(const unsigned int axis_a, const unsigned int axis_b) const;

  /// The degree of freedom this kernel acts on: 0 to 2 for the displacements, 3 to 5 for the rotations
  const unsigned int _component;

  /// Whether the rotations in the plane of the element carry rotary inertia
  const bool _rotary_inertia;

  /// Variable numbers of the six degrees of freedom, in the order of the components
  std::vector<unsigned int> _variable_numbers;

  /// Second time derivatives of the six degrees of freedom
  std::vector<const VariableValue *> _accelerations;

  /// First time derivatives of the six degrees of freedom
  std::vector<const VariableValue *> _velocities;

  /// First time derivatives of the six degrees of freedom at the previous time step
  std::vector<const VariableValue *> _old_velocities;

  /// Derivatives of the second time derivatives with respect to the degrees of freedom
  std::vector<const VariableValue *> _acceleration_derivatives;

  /// Derivatives of the first time derivatives with respect to the degrees of freedom
  std::vector<const VariableValue *> _velocity_derivatives;

  /// Density of the shell material
  const MaterialProperty<Real> & _density;

  /// Thickness of the shell, from the section stiffness material
  const MaterialProperty<Real> & _thickness;

  /// Rotation matrix of the element frame, from the strain material
  const MaterialProperty<RankTwoTensor> & _rotation_matrix;

  /// Mass-proportional Rayleigh damping coefficient
  const MaterialProperty<Real> & _eta;

  /// HHT time integration parameter
  const Real _alpha;
};
