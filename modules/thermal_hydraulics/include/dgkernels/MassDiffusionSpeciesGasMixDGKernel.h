//
// This file is part of Sockeye heat pipe performance code.
// All rights reserved; see COPYRIGHT for full restrictions.
//

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
