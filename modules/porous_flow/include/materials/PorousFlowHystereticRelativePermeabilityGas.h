//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowHystereticRelativePermeabilityBase.h"

/**
 * Material to compute gas relative permeability for 1-phase and 2-phase hysteretic models
 */
class PorousFlowHystereticRelativePermeabilityGas
  : public PorousFlowHystereticRelativePermeabilityBase
{
public:
  static InputParameters validParams();

  PorousFlowHystereticRelativePermeabilityGas(const InputParameters & parameters);

protected:
  /// Phase number of liquid phase
  const unsigned _liquid_phase;

  /// gamma parameter used in the gas relative permeability
  const Real _gamma;

  /// Value of the  gas relative permeability at liquid saturation = _s_lr
  const Real _k_rg_max;

  /// Value of the derivative of the cubic extension to the gas relative permeability at liquid saturation = _s_lr
  const Real _krel_gas_prime;

  virtual void computeRelPermQp() override;
};
