#pragma once

#include "ADOneDHeatFluxBase.h"

class ADOneD3EqnEnergyHeatFlux : public ADOneDHeatFluxBase
{
public:
  ADOneD3EqnEnergyHeatFlux(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

public:
  static InputParameters validParams();
};
