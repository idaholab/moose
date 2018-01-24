/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NEWMARKACCELAUX_H
#define NEWMARKACCELAUX_H

#include "AuxKernel.h"

class NewmarkAccelAux;

template <>
InputParameters validParams<NewmarkAccelAux>();

class NewmarkAccelAux : public AuxKernel
{
public:
  /**
  *Computes Acceleration using Newmark Time integration scheme
  */
  NewmarkAccelAux(const InputParameters & parameters);

  virtual ~NewmarkAccelAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _disp_old;
  const VariableValue & _disp;
  const VariableValue & _vel_old;
  Real _beta;
};

#endif // NEWMARKACCELAUX_H
