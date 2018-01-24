//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSDIVERGENCEAUX_H
#define INSDIVERGENCEAUX_H

#include "AuxKernel.h"

// Forward Declarations
class INSDivergenceAux;

template <>
InputParameters validParams<INSDivergenceAux>();

/**
 * Computes h_min / |u|
 */
class INSDivergenceAux : public AuxKernel
{
public:
  INSDivergenceAux(const InputParameters & parameters);

  virtual ~INSDivergenceAux() {}

protected:
  virtual Real computeValue();

  // Velocity gradients
  const VariableGradient & _grad_u_vel;
  const VariableGradient & _grad_v_vel;
  const VariableGradient & _grad_w_vel;
};

#endif // VELOCITYAUX_H
