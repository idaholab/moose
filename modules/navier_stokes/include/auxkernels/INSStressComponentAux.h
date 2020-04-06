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
class INSStressComponentAux : public AuxKernel
{
public:
  static InputParameters validParams();

  INSStressComponentAux(const InputParameters & parameters);

  virtual ~INSStressComponentAux() {}

protected:
  virtual Real computeValue();

  // Velocity gradients
  const VariableGradient & _grad_velocity;
  const VariableValue & _pressure;
  const unsigned _comp;
  const MaterialProperty<Real> & _mu;
};
