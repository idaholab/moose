#pragma once

#include "DiffusionFluxAux.h"

/**
 * Computes linear heat rate
 */
class QPrimeAuxPin : public DiffusionFluxAux
{
public:
  static InputParameters validParams();

  QPrimeAuxPin(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  /// rod diameter
  const Real & _rod_diameter;
};
