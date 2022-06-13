#pragma once

#include "DiffusionFluxAux.h"

/**
 * Computes linear heat rate
 */
class QPrimeDuctAux : public DiffusionFluxAux
{
public:
  static InputParameters validParams();

  QPrimeDuctAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  /// rod diameter
  const Real & _flat_to_flat;
};
