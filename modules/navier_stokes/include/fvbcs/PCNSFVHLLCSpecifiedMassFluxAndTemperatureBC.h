#pragma once

#include "PCNSFVHLLCBC.h"

class Function;

class PCNSFVHLLCSpecifiedMassFluxAndTemperatureBC : public PCNSFVHLLCBC
{
public:
  PCNSFVHLLCSpecifiedMassFluxAndTemperatureBC(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  void preComputeWaveSpeed() override;

  const Function & _superficial_rhou_boundary;
  const Function * const _superficial_rhov_boundary;
  const Function * const _superficial_rhow_boundary;
  const Function & _temperature_boundary;
};
