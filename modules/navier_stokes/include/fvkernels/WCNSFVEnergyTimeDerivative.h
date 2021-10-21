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
 * Computes the energy time derivative for the weakly compressible formulation of the energy
 * equation, using functor material properties
 */
class WCNSFVEnergyTimeDerivative : public FVTimeKernel
{
public:
  static InputParameters validParams();
  WCNSFVEnergyTimeDerivative(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// Density functor, material property or variable
  const Moose::Functor<ADReal> & _rho;

  /// Specific heat functor, material property or variable
  const Moose::Functor<ADReal> & _cp;

  /// Functor for the time derivative of density, material property or variable
  const Moose::Functor<ADReal> & _rho_dot;

  /// Functor for the time derivative of the specific heat, material property or variable
  const Moose::Functor<ADReal> & _cp_dot;
};
