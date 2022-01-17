#pragma once

#include "ElementIntegralPostprocessor.h"

/**
 * Base class for computing the total energy for heat structures
 */
class ADHeatStructureEnergyBase : public ElementIntegralPostprocessor
{
public:
  ADHeatStructureEnergyBase(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Number of units that heat structure is multiplied by
  const Real _n_units;

  /// Reference temperature
  const Real & _T_ref;

  /// Density of the heat structure
  const ADMaterialProperty<Real> & _rho;

  /// Isobaric specific heat capacity
  const ADMaterialProperty<Real> & _cp;

  /// Temperature variable
  MooseVariable * _T_var;
  /// Temperature variable value
  const VariableValue & _T;

public:
  static InputParameters validParams();
};
