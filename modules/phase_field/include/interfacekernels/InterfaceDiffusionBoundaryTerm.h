/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INTERFACEDIFFUSIONBOUNDARYTERM_H
#define INTERFACEDIFFUSIONBOUNDARYTERM_H

#include "InterfaceDiffusionBase.h"

class InterfaceDiffusionBoundaryTerm;

template <>
InputParameters validParams<InterfaceDiffusionBoundaryTerm>();

/**
 * Add weak form surface terms of the Diffusion equation for two different
 * variables across a subdomain boundary
 */
class InterfaceDiffusionBoundaryTerm : public InterfaceDiffusionBase
{
public:
  InterfaceDiffusionBoundaryTerm(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
};

#endif // INTERFACEDIFFUSIONBOUNDARYTERM_H
