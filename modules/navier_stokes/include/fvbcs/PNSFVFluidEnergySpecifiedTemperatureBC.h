#pragma once

#include "FVFluxBC.h"

class Function;
class SinglePhaseFluidProperties;

/**
 * This boundary provides the advective flux of fluid energy across a boundary
 * given a specified temperature
 */
class PNSFVFluidEnergySpecifiedTemperatureBC : public FVFluxBC
{
public:
  PNSFVFluidEnergySpecifiedTemperatureBC(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  const Function & _temperature;
  const Function & _superficial_rhou;
  const Function * const _superficial_rhov;
  const Function * const _superficial_rhow;
  const ADMaterialProperty<Real> & _rho_elem;
  const ADMaterialProperty<Real> & _rho_neighbor;
  const MaterialProperty<Real> & _porosity_elem;
  const MaterialProperty<Real> & _porosity_neighbor;

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;
};
