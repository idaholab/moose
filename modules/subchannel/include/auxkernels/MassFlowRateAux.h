#pragma once

#include "AuxKernel.h"

/**
 * Computes mass float rate from specified uniform mass flux and cross-sectional area
 */
class MassFlowRateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  MassFlowRateAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  /// Specified mass flux
  const Real & _mass_flux;
  /// Cross-sectional area
  const VariableValue & _area;
};
