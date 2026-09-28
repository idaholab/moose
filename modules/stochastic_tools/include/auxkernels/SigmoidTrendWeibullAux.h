//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SigmoidTrendAuxBase.h"

/**
 * Weibull random field via Gaussian copula whose scale trends with distance from a line segment
 */
class SigmoidTrendWeibullAux : public SigmoidTrendAuxBase
{
public:
  static InputParameters validParams();
  SigmoidTrendWeibullAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Spatially constant Weibull shape parameter k
  const Real _shape;
};
