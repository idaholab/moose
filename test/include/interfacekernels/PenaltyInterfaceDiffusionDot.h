//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceTimeKernel.h"

/**
 * Interface kernel enforcing continuity of flux and continuity of time derivatives
 */
class PenaltyInterfaceDiffusionDot : public InterfaceTimeKernel
{
public:
  static InputParameters validParams();

  PenaltyInterfaceDiffusionDot(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  const Real _penalty;
};
