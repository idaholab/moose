#pragma once

#include "FVFluxBC.h"

class Function;
class SinglePhaseFluidProperties;

/**
 * This boundary provides the advective flux of fluid energy across a boundary
 * given a specified temperature
 */
class NSFVFluidEnergySpecifiedTemperatureBC : public FVFluxBC
{
public:
  NSFVFluidEnergySpecifiedTemperatureBC(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  const Function & _temperature;
  const Function & _rhou;
  const Function * const _rhov;
  const Function * const _rhow;
  const ADMaterialProperty<Real> & _rho_elem;
  const ADMaterialProperty<Real> & _rho_neighbor;

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;

  /// The interpolation method to use
  Moose::FV::InterpMethod _interp_method;
};
