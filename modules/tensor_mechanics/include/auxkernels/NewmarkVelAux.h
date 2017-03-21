/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
