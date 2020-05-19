//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeNodalKernel.h"
#include "RankTwoTensor.h"

// Forward Declarations
class TimeIntegrator;

/**
 * Calculates the inertial torque and inertia proportional damping
 * for nodal rotational inertia
 */
class NodalRotationalInertia : public TimeNodalKernel
{
public:
  static InputParameters validParams();

  NodalRotationalInertia(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Booleans for validity of params
  const bool _has_beta;
  const bool _has_gamma;
  const bool _has_rot_velocities;
  const bool _has_rot_accelerations;
  const bool _has_x_orientation;
  const bool _has_y_orientation;

  /// Auxiliary system object
  AuxiliarySystem * _aux_sys;

  /// Number of coupled rotational variables
  unsigned int _nrot;

  /// Value of rotational displacements
  std::vector<const VariableValue *> _rot;

  /// Old value of rotational displacements
  std::vector<const VariableValue *> _rot_old;

  /// Variable numbers for rotational velocity aux variables
  std::vector<unsigned int> _rot_vel_num;

  /// Variable numbers for rotational acceleration aux variables
  std::vector<unsigned int> _rot_accel_num;

  /// Variable numbers for rotational variables
  std::vector<unsigned int> _rot_variables;

  /// Current acceleration of the node
  std::vector<Real> _rot_accel;

  /// Current velocity of the node
  std::vector<Real> _rot_vel;

  /// Old velocity of the node
  std::vector<Real> _rot_vel_old;

  /// Newmark time integration parameter
  const Real _beta;

  /// Newmark time integration parameter
  const Real _gamma;

  /// Mass proportional Rayliegh damping
  const Real & _eta;

  /// HHT time integration parameter
  const Real & _alpha;

  /// Component along which torque is applied
  const unsigned int _component;

  /// Moment of inertia tensor in global coordinate system
  RankTwoTensor _inertia;

  /// Rotational udot residual
  std::vector<const VariableValue *> _rot_dot_residual;

  /// Old velocity value
  std::vector<const VariableValue *> _rot_vel_old_value;

  /// Rotational udotdot residual
  std::vector<const VariableValue *> _rot_dotdot_residual;

  /// du_dot_du value
  const VariableValue * _du_dot_du;

  /// du_dotdot_du value
  const VariableValue * _du_dotdot_du;

  /// The TimeIntegrator
  TimeIntegrator & _time_integrator;
};
