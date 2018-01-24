/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INTERFACEDIFFUSIONFLUXMATCH_H
#define INTERFACEDIFFUSIONFLUXMATCH_H

#include "InterfaceDiffusionBase.h"

class InterfaceDiffusionFluxMatch;

template <>
InputParameters validParams<InterfaceDiffusionFluxMatch>();

/**
 * Enforce gradient continuity between two different variables across a
 * subdomain boundary.
 */
class InterfaceDiffusionFluxMatch : public InterfaceDiffusionBase
{
public:
  InterfaceDiffusionFluxMatch(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
};

#endif // INTERFACEDIFFUSIONFLUXMATCH_H
