#pragma once

#include "CNSFVHLLCBC.h"

class Function;

class CNSFVHLLCSpecifiedMassFluxAndTemperatureBC : public CNSFVHLLCBC
{
public:
  CNSFVHLLCSpecifiedMassFluxAndTemperatureBC(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  void preComputeWaveSpeed() override;

  const Function & _rhou_boundary;
  const Function * const _rhov_boundary;
  const Function * const _rhow_boundary;
  const Function & _temperature_boundary;
};
