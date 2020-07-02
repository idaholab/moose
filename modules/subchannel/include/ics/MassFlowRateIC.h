#pragma once

#include "InitialCondition.h"

/**
 * Computes mass float rate from specifiec mass flux and cross-sectional area
 */
class MassFlowRateIC : public InitialCondition
{
public:
  static InputParameters validParams();

  MassFlowRateIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// Specified mass flux
  const Real & _mass_flux;
  /// Cross-sectional area
  const VariableValue & _area;
};
