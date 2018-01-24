//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EXPLICITODE_H_
#define EXPLICITODE_H_

#include "AuxScalarKernel.h"

// Forward Declarations
class ExplicitODE;

template <>
InputParameters validParams<ExplicitODE>();

/**
 * Explicit solve of ODE:
 *
 * dy/dt = -\lambda y  (using forward Euler)
 */
class ExplicitODE : public AuxScalarKernel
{
public:
  ExplicitODE(const InputParameters & parameters);
  virtual ~ExplicitODE();

protected:
  virtual Real computeValue();

  Real _lambda;
};

#endif /* EXPLICITODE_H_ */
