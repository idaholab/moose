//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "SinglePhaseFluidProperties.h"
#include "MooseVariable.h"

/**
 * This class couples together all the variables for the 3D fluid equations to allow them to be used
 * in derived Kernel classes.
 */
class MDFluidKernelBase : public Kernel
{
public:
  static InputParameters validParams();

  MDFluidKernelBase(const InputParameters & parameters);
  virtual ~MDFluidKernelBase() {}

protected:
  Real velocityDiv() { return _grad_u_vel[_qp](0) + _grad_v_vel[_qp](1) + _grad_w_vel[_qp](2); }
  RealVectorValue velocityDot()
  {
    RealVectorValue vec_vel_dot(_u_vel_dot[_qp], _v_vel_dot[_qp], _w_vel_dot[_qp]);
    return vec_vel_dot;
  }

  const VariableSecond & _second_u;
  // Coupled variables
  MooseVariable & _u_var;
  MooseVariable & _v_var;
  MooseVariable & _w_var;

  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _pressure;
  const VariableValue & _temperature;

  const MaterialProperty<Real> & _rho;
  bool _has_porosity;
  const VariableValue & _porosity;

  bool _bTransient;
  const VariableValue & _u_vel_dot;
  const VariableValue & _v_vel_dot;
  const VariableValue & _w_vel_dot;

  // Gradients
  const VariableGradient & _grad_u_vel;
  const VariableGradient & _grad_v_vel;
  const VariableGradient & _grad_w_vel;
  const VariableGradient & _grad_pressure;
  const VariableGradient & _grad_temperature;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;
  unsigned _pressure_var_number;
  unsigned _temperature_var_number;

  // Material properties
  const MaterialProperty<RealTensorValue> & _viscous_stress_tensor;
  const MaterialProperty<Real> & _dynamic_viscosity;
  const MaterialProperty<Real> & _turbulence_viscosity;
  const MaterialProperty<RealTensorValue> & _inertia_resistance_coeff;
  const MaterialProperty<RealTensorValue> & _viscous_resistance_coeff;

  // Helper function for mapping Moose variable numberings into
  // the "canonical" numbering for the porous medium equations.
  unsigned map_var_number(unsigned var);

  const SinglePhaseFluidProperties & _eos;

  RealVectorValue _vec_g;
};
