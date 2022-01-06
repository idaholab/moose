//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVTimeKernel.h"

/**
 * Computes the scalar time derivative for the weakly compressible formulation of the scalar
 * transport equation, using functor material properties
 */
class WCNSFVScalarTimeDerivative : public FVTimeKernel
{
public:
  static InputParameters validParams();
  WCNSFVScalarTimeDerivative(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// Density functor
  const Moose::Functor<ADReal> & _rho;

  /// Functor for the time derivative of density
  const Moose::Functor<ADReal> & _rho_dot;
};
