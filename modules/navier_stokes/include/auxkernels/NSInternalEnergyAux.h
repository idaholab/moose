/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSINTERNALENERGYAUX_H
#define NSINTERNALENERGYAUX_H

#include "AuxKernel.h"

// Forward Declarations
class NSInternalEnergyAux;

template <>
InputParameters validParams<NSInternalEnergyAux>();

/**
 * Auxiliary kernel for computing the internal energy of the fluid.
 */
class NSInternalEnergyAux : public AuxKernel
{
public:
  NSInternalEnergyAux(const InputParameters & parameters);

  virtual ~NSInternalEnergyAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _rhoE;
};

#endif
