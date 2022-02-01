#pragma once

#include "DiffusionFluxAux.h"

/**
 * Computes linear heat rate
 */
class QPrimeAux : public DiffusionFluxAux
{
public:
  static InputParameters validParams();

  QPrimeAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  /// rod diameter
  const Real & _rod_diameter;
};
