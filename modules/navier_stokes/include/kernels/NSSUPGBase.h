//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSKernel.h"

// Forward Declarations

/**
 * This class acts as a base class for stabilization kernels.
 * This is useful because the stabilization kernels for different
 * equations share a lot of information...
 */
class NSSUPGBase : public NSKernel
{
public:
  static InputParameters validParams();

  NSSUPGBase(const InputParameters & parameters);

protected:
  // Material properties
  const MaterialProperty<RealTensorValue> & _viscous_stress_tensor;
  const MaterialProperty<Real> & _dynamic_viscosity;
  const MaterialProperty<Real> & _thermal_conductivity;

  // SUPG-related material properties.
  const MaterialProperty<Real> & _hsupg;
  const MaterialProperty<Real> & _tauc;
  const MaterialProperty<Real> & _taum;
  const MaterialProperty<Real> & _taue;
  const MaterialProperty<std::vector<Real>> & _strong_residuals;

  // Momentum equation inviscid flux matrices
  const MaterialProperty<std::vector<RealTensorValue>> & _calA;

  // "velocity column" matrices
  const MaterialProperty<std::vector<RealTensorValue>> & _calC;

  // Energy equation inviscid flux matrices
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _calE;

  // "Old" (from previous timestep) coupled variable values.
  // const VariableValue & _rho_old;
  // const VariableValue & _rho_u_old;
  // const VariableValue & _rho_v_old;
  // const VariableValue & _rho_w_old;
  // const VariableValue & _rho_e_old;

  // The derivative of "udot" wrt u for each of the momentum variables.
  // This is always 1/dt unless you are using BDF2...
  const VariableValue & _d_rhodot_du;
  const VariableValue & _d_rhoudot_du;
  const VariableValue & _d_rhovdot_du;
  const VariableValue & _d_rhowdot_du;
  const VariableValue & _d_rho_etdot_du;

  // Temperature is need to compute speed of sound
  const VariableValue & _temperature;

  // Enthalpy aux variable
  const VariableValue & _specific_total_enthalpy;
};
