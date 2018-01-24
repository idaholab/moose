//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
