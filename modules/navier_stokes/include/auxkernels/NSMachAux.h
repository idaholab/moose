/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSMACHAUX_H
#define NSMACHAUX_H

#include "AuxKernel.h"

//Forward Declarations
class NSMachAux;

template<>
InputParameters validParams<NSMachAux>();

/**
 * Auxiliary kernel for computing the Mach number assuming an ideal gas.
 */
class NSMachAux : public AuxKernel
{
public:
  NSMachAux(const InputParameters & parameters);

  virtual ~NSMachAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _temperature;

  Real _gamma;
  Real _R;
};

#endif
