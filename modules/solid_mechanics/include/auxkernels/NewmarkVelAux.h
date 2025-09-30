//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Calcualtes velocity using Newmark time integration scheme
 */
class NewmarkVelAux : public AuxKernel
{
public:
  static InputParameters validParams();

  NewmarkVelAux(const InputParameters & parameters);

  virtual ~NewmarkVelAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _accel_old;
  const VariableValue & _accel;
  const VariableValue & _u_old;
  Real _gamma;
};
