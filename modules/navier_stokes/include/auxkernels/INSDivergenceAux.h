//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

// Forward Declarations

/**
 * Computes h_min / |u|
 */
class INSDivergenceAux : public AuxKernel
{
public:
  static InputParameters validParams();

  INSDivergenceAux(const InputParameters & parameters);

  virtual ~INSDivergenceAux() {}

protected:
  virtual Real computeValue();

  // Velocity gradients
  const VariableGradient & _grad_u_vel;
  const VariableGradient & _grad_v_vel;
  const VariableGradient & _grad_w_vel;
};
