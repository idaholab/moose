/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EXPVELAUX_H
#define EXPVELAUX_H

#include "AuxKernel.h"

class ExpVelAux;

template <>
InputParameters validParams<ExpVelAux>();

class ExpVelAux : public AuxKernel
{
public:
  /**
   * Calcualtes velocity using Newmark time integration scheme
   */
  ExpVelAux(const InputParameters & parameters);

  virtual ~ExpVelAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _disp_older;
  const VariableValue & _disp_old;
  const VariableValue & _disp;
};

#endif // EXPVELAUX_H
