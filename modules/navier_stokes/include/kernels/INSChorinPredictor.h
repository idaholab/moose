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
 * This class computes the "Chorin" Predictor equation in fully-discrete
 * (both time and space) form.
 */
class INSChorinPredictor : public Kernel
{
public:
  static InputParameters validParams();

  INSChorinPredictor(const InputParameters & parameters);

  virtual ~INSChorinPredictor() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Velocity
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;

  // Old Velocity
  const VariableValue & _u_vel_old;
  const VariableValue & _v_vel_old;
  const VariableValue & _w_vel_old;

  // Star Velocity
  const VariableValue & _u_vel_star;
  const VariableValue & _v_vel_star;
  const VariableValue & _w_vel_star;

  // Velocity Gradients
  const VariableGradient & _grad_u_vel;
  const VariableGradient & _grad_v_vel;
  const VariableGradient & _grad_w_vel;

  // Old Velocity Gradients
  const VariableGradient & _grad_u_vel_old;
  const VariableGradient & _grad_v_vel_old;
  const VariableGradient & _grad_w_vel_old;

  // Star Velocity Gradients
  const VariableGradient & _grad_u_vel_star;
  const VariableGradient & _grad_v_vel_star;
  const VariableGradient & _grad_w_vel_star;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;

  // Star velocity numbers
  unsigned _u_vel_star_var_number;
  unsigned _v_vel_star_var_number;
  unsigned _w_vel_star_var_number;

  // Parameters
  unsigned _component;

  // An enumeration defining which velocity vector is used on the rhs
  // of the Chorin predictor.
  // OLD  - Use velocity from the previous timestep, leads to explicit method
  // NEW  - Use velocity from current timestep, this may not be an actual method
  // STAR - Use the "star" velocity.  According to Donea's book, this is the
  //        right way to get an implicit method...
  MooseEnum _predictor_enum;

  // A C++ enumeration corresponding to the predictor enumeration.
  enum PredictorType
  {
    OLD = 0,
    NEW = 1,
    STAR = 2
  };

  // Material properties
  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _rho;
};
