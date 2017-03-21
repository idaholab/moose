/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INTERFACEDIFFUSIONBASE_H
#define INTERFACEDIFFUSIONBASE_H

#include "InterfaceKernel.h"

class InterfaceDiffusionBase;

template <>
InputParameters validParams<InterfaceDiffusionBase>();

/**
 * Base class for Diffusion equation terms coupling two different
 * variables across a subdomain boundary.
 */
class InterfaceDiffusionBase : public InterfaceKernel
{
public:
  InterfaceDiffusionBase(const InputParameters & parameters);

protected:
  /// diffusion coefficient
  const Real _D;

  /// neighbor diffusion coefficient
  const Real _D_neighbor;
};

#endif // INTERFACEDIFFUSIONBASE_H
