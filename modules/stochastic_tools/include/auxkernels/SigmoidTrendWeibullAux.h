#pragma once

#include "SigmoidTrendAuxBase.h"

class SigmoidTrendWeibullAux : public SigmoidTrendAuxBase
{
public:
  static InputParameters validParams();
  SigmoidTrendWeibullAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const Real _shape;
};
