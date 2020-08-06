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

/**
 * Computes the inertial torque, which is
 * density * displacement x acceleration
 * (a cross-product is used).  Newmark time
 * integration is used, and parameters are included
 * that allow Rayleigh damping and HHT time integration
 */
class InertialTorque : public TimeKernel
{
public:
  static InputParameters validParams();

  InertialTorque(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  /// density
  const MaterialProperty<Real> & _density;

  /// Newmark beta parameter
  const Real _beta;

  /// Newmark gamma parameter
  const Real _gamma;

  /// Rayleigh-damping eta parameter
  const MaterialProperty<Real> & _eta;

  /// HHT alpha parameter
  const Real _alpha;

  /**
   * Component of the cross-product desired.  This
   * kernel will calculate the _component component
   * of density * displacement x acceleration
   */
  const unsigned _component;

  /// Number of displacement variables.  This must be 3
  const unsigned _ndisp;

  /// MOOSE internal variable numbers corresponding to the displacments
  const std::vector<unsigned> _disp_num;

  /// Displacements
  const std::vector<const VariableValue *> _disp;

  /// Old value of displacements
  const std::vector<const VariableValue *> _disp_old;

  /// Old value of velocities
  const std::vector<const VariableValue *> _vel_old;

  /// Old value of accelerations
  const std::vector<const VariableValue *> _accel_old;

  /// Acceleration (instantiating this vector avoids re-creating a new vector every residual calculation)
  std::vector<Real> _accel;

  /// Velocity (instantiating this vector avoids re-creating a new vector every residual calculation)
  std::vector<Real> _vel;

  /// Derivative of acceleration with respect to displacement
  std::vector<Real> _daccel;

  /// Derivative of velocity with respect to displacement
  std::vector<Real> _dvel;
};
