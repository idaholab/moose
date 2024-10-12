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
#include "ADTimeKernel.h"
#include "Material.h"

// Forward Declarations
class TimeIntegrator;

// parent class
template <bool is_ad>
using InertialForceParent = typename std::conditional<is_ad, ADTimeKernel, TimeKernel>::type;

template <bool is_ad>
class InertialForceTempl : public InertialForceParent<is_ad>
{
public:
  static InputParameters validParams();

  InertialForceTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual();

  virtual Real computeQpJacobian();

  virtual void computeResidualAdditional();

private:
  const GenericMaterialProperty<Real, is_ad> & _density;
  const VariableValue * _u_old;
  const VariableValue * _vel_old;
  const VariableValue * _accel_old;
  const bool _has_beta;
  const bool _has_gamma;
  const Real _beta;
  const Real _gamma;
  const bool _has_velocity;
  const bool _has_acceleration;
  const GenericMaterialProperty<Real, is_ad> & _eta;
  const MaterialProperty<Real> & _density_scaling;
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
  const TimeIntegrator & _time_integrator;

  using InertialForceParent<is_ad>::_dt;
  using InertialForceParent<is_ad>::_i;
  using InertialForceParent<is_ad>::_phi;
  using InertialForceParent<is_ad>::_qp;
  using InertialForceParent<is_ad>::_sys;
  using InertialForceParent<is_ad>::_test;
  using InertialForceParent<is_ad>::_u;
  using InertialForceParent<is_ad>::_var;
};

using InertialForce = InertialForceTempl<false>;
using ADInertialForce = InertialForceTempl<true>;
