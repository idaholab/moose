//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class KLExpansionUserObject;

/**
 * Transforms a KLExpansionUserObject's correlated Gaussian field into a
 * correlated Weibull-distributed field via a Gaussian copula and inverse transform
 * Z(p) -> standardize -> Phi(.) -> Uniform(0,1) -> Weibull inverse CDF
 */
class KLWeibullAux : public AuxKernel
{
public:
  static InputParameters validParams();
  KLWeibullAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const KLExpansionUserObject & _kl_uo;
  /// Weibull shape parameter k
  const Real _shape;
  /// Weibull scale parameter lambda
  const Real _scale;
};
