/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EXPACCELAUX_H
#define EXPACCELAUX_H

#include "AuxKernel.h"

class ExpAccelAux;

template <>
InputParameters validParams<ExpAccelAux>();

class ExpAccelAux : public AuxKernel
{
public:
  /**
  *Computes Acceleration using Newmark Time integration scheme
  */
  ExpAccelAux(const InputParameters & parameters);

  virtual ~ExpAccelAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _disp_older;
  const VariableValue & _disp_old;
  const VariableValue & _disp;
};

#endif // EXPACCELAUX_H
