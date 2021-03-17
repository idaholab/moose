#pragma once

#include "PCNSFVMomentumAdvectionSpecifiedMassFluxBC.h"

class Function;
class SinglePhaseFluidProperties;

/**
 * This boundary provides the advective flux of fluid energy across a boundary
 * given a specified temperature
 */
class PCNSFVFluidEnergyAdvectionSpecifiedMassFluxTemperatureBC
  : public PCNSFVMomentumAdvectionSpecifiedMassFluxBC
{
public:
  PCNSFVFluidEnergyAdvectionSpecifiedMassFluxTemperatureBC(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  const Function & _temperature;

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;
};
