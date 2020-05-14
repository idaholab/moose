//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeKernel.h"
#include "Material.h"

// Forward Declarations
class TimeIntegrator;

class InertialForce : public TimeKernel
{
public:
  static InputParameters validParams();

  InertialForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual void computeResidualAdditional() override;

private:
  const MaterialProperty<Real> & _density;
  const VariableValue * _u_old;
  const VariableValue * _vel_old;
  const VariableValue * _accel_old;
  const bool _has_beta;
  const bool _has_gamma;
  const Real _beta;
  const Real _gamma;
  const bool _has_velocity;
  const bool _has_acceleration;
  const MaterialProperty<Real> & _eta;
  const Real _alpha;

  // Velocity and acceleration calculated by time integrator
  const VariableValue * _u_dot_factor_dof;
  const VariableValue * _u_dotdot_factor_dof;
  const VariableValue * _u_dot_factor;
  const VariableValue * _u_dotdot_factor;
  const VariableValue * _u_dot_old;
  const VariableValue * _du_dot_du;
  const VariableValue * _du_dotdot_du;

  /// The TimeIntegrator
  TimeIntegrator & _time_integrator;
};
