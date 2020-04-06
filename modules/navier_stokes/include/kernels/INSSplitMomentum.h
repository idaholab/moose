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

// Forward Declarations

/**
 * This class computes the "split" momentum equation residual.  In the
 * split method, this is a time-independent vector equation for "a",
 * an intermediate "acceleration" vector.  The pressure is not coupled
 * directly to momentum in the split method.  Note: this equation is
 * divided through by the density, so "nu" appears rather than "mu", for
 * instance.
 *
 * Do not use, USE INSChorinPredictor and related classes instead.
 */
class INSSplitMomentum : public Kernel
{
public:
  static InputParameters validParams();

  INSSplitMomentum(const InputParameters & parameters);

  virtual ~INSSplitMomentum() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled variables
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;

  // Acceleration vector components
  const VariableValue & _a1;
  const VariableValue & _a2;
  const VariableValue & _a3;

  // Gradients
  const VariableGradient & _grad_u_vel;
  const VariableGradient & _grad_v_vel;
  const VariableGradient & _grad_w_vel;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;

  unsigned _a1_var_number;
  unsigned _a2_var_number;
  unsigned _a3_var_number;

  // Parameters
  RealVectorValue _gravity;
  unsigned _component;

  // Material properties
  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _rho;
};
