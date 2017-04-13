/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVMACHAUX_H
#define CNSFVMACHAUX_H

#include "AuxKernel.h"

class CNSFVMachAux;

template <>
InputParameters validParams<CNSFVMachAux>();

/**
 * An aux kernel for calculating Mach number
 */
class CNSFVMachAux : public AuxKernel
{
public:
  CNSFVMachAux(const InputParameters & parameters);
  virtual ~CNSFVMachAux() {}

protected:
  virtual Real computeValue();

  const MaterialProperty<Real> & _mach;
};

#endif
