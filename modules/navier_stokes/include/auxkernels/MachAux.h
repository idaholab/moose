#pragma once

#include "AuxKernel.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Auxiliary kernel to compute the mach number from specific
 * pressure & temperature (as matprops) and fluid props object
 */
class MachAux : public AuxKernel
{
public:
  MachAux(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual Real computeValue();

  /// speed
  const ADMaterialProperty<Real> & _speed;

  /// pressure
  const ADMaterialProperty<Real> & _pressure;

  /// temperature
  const ADMaterialProperty<Real> & _temperature;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fluid;
};
