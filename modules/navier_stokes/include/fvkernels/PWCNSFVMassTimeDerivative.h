//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVMassTimeDerivative.h"

/**
 * Computes the mass time derivative for the weakly compressible formulation of the mass
 * equation, using functor material properties and a porous medium approach
 */
class PWCNSFVMassTimeDerivative : public WCNSFVMassTimeDerivative
{
public:
  static InputParameters validParams();
  PWCNSFVMassTimeDerivative(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  const Moose::Functor<ADReal> & _eps;
};
