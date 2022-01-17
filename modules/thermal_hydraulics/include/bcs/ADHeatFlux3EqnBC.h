#pragma once

#include "ADHeatFluxBaseBC.h"

class ADHeatFlux3EqnBC : public ADHeatFluxBaseBC
{
public:
  ADHeatFlux3EqnBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

public:
  static InputParameters validParams();
};
