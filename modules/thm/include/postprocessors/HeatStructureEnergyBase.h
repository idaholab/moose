#pragma once

#include "ElementIntegralPostprocessor.h"

class HeatStructureEnergyBase;

template <>
InputParameters validParams<HeatStructureEnergyBase>();

/**
 * Base class for computing the total energy for heat structures
 */
class HeatStructureEnergyBase : public ElementIntegralPostprocessor
{
public:
  HeatStructureEnergyBase(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Number of units that heat structure is multiplied by
  const unsigned int _n_units;

  /// Reference temperature
  const Real & _T_ref;

  /// Density of the heat structure
  const MaterialProperty<Real> & _rho;

  /// Isobaric specific heat capacity
  const MaterialProperty<Real> & _cp;

  /// Temperature variable
  MooseVariable * _T_var;
  /// Temperature variable value
  const VariableValue & _T;
};
