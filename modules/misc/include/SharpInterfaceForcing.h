/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SHARPINTERFACEFORCING_H
#define SHARPINTERFACEFORCING_H

#include "Kernel.h"
#include "Function.h"

class SharpInterfaceForcing;

template <>
InputParameters validParams<SharpInterfaceForcing>();

/**
 * Note: Useful class for testing
 */
class SharpInterfaceForcing : public Kernel
{
public:
  SharpInterfaceForcing(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  Function & _x_center;
  Function & _y_center;
  Real _amplitude;
};

#endif // SHARPINTERFACEFORCING_H
