/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INTERFACEDIFFUSIONFLUX_H
#define INTERFACEDIFFUSIONFLUX_H

#include "InterfaceKernel.h"

class InterfaceDiffusionFlux;

template<>
InputParameters validParams<InterfaceDiffusionFlux>();

/**
 * Add weak form surface terms of the Diffusion equation for two different
 * variables across a subdomain boundary
 */
class InterfaceDiffusionFlux : public InterfaceKernel
{
public:
  InterfaceDiffusionFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  /// diffusion coefficient
  const Real _D;

  /// neighbor diffusion coefficient
  const Real _D_neighbor;
};

#endif // INTERFACEDIFFUSIONFLUX_H
