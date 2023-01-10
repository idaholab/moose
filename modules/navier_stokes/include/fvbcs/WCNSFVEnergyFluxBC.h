//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVFluxBCBase.h"

/**
 * Flux boundary condition for the weakly compressible energy equation
 */
class WCNSFVEnergyFluxBC : public WCNSFVFluxBCBase
{
public:
  static InputParameters validParams();
  WCNSFVEnergyFluxBC(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// Postprocessor with the inlet temperature
  const PostprocessorValue * const _temperature_pp;

  /// Postprocessor with the inlet energy flow rate
  const PostprocessorValue * const _energy_pp;

  /// Fluid specific heat capacity functor
  const Moose::Functor<ADReal> * const _cp;
};
