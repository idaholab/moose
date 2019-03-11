//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSSTRESSCOMPONENTAUX_H
#define INSSTRESSCOMPONENTAUX_H

#include "AuxKernel.h"

// Forward Declarations
class INSStressComponentAux;

template <>
InputParameters validParams<INSStressComponentAux>();

/**
 * Computes h_min / |u|
 */
class INSStressComponentAux : public AuxKernel
{
public:
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

#endif // VELOCITYAUX_H
