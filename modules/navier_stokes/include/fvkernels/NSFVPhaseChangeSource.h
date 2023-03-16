//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

/**
 * Heat source coming from the melting/solidification of materials
 */
class NSFVPhaseChangeSource : public FVElementalKernel
{
public:
  NSFVPhaseChangeSource(const InputParameters & params);
  static InputParameters validParams();

protected:
  ADReal computeQpResidual() override;

  /// The functor describing the body force
  const Moose::Functor<ADReal> & _liquid_fraction;

  /// Latent heat
  const Moose::Functor<ADReal> & _L;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Solidus Temperature
  const Moose::Functor<ADReal> & _T_solidus;

  /// Liquidus Temperature
  const Moose::Functor<ADReal> & _T_liquidus;
};
