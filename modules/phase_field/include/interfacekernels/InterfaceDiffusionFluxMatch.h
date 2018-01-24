//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
