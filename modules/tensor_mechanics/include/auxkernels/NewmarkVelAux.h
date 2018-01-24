//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NEWMARKVELAUX_H
#define NEWMARKVELAUX_H

#include "AuxKernel.h"

class NewmarkVelAux;

template <>
InputParameters validParams<NewmarkVelAux>();

class NewmarkVelAux : public AuxKernel
{
public:
  /**
   * Calcualtes velocity using Newmark time integration scheme
   */
  NewmarkVelAux(const InputParameters & parameters);

  virtual ~NewmarkVelAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _accel_old;
  const VariableValue & _accel;
  Real _gamma;
};

#endif // NEWMARKVELAUX_H
