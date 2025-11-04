//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MassDiffusionBaseGasMixDGKernel.h"

/**
 * Adds mass diffusion to the species equation for FlowChannelGasMix.
 */
class MassDiffusionSpeciesGasMixDGKernel : public MassDiffusionBaseGasMixDGKernel
{
public:
  static InputParameters validParams();

  MassDiffusionSpeciesGasMixDGKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpFlux() const override;
};
