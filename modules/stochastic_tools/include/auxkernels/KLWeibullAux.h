#pragma once

#include "AuxKernel.h"

// Transforms a KLExpansionUserObject's correlated Gaussian field into a
// correlated Weibull-distributed field via a Gaussian copula and inverse transform
// Z(p) -> standardize -> Phi(.) -> Uniform(0,1) -> Weibull inverse CDF
class KLExpansionUserObject;

class KLWeibullAux : public AuxKernel
{
public:
  static InputParameters validParams();
  KLWeibullAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const KLExpansionUserObject & _kl_uo;
  const Real _shape; // Weibull shape parameter k
  const Real _scale; // Weibull scale parameter lambda
  // https://en.wikipedia.org/wiki/Weibull_distribution
};
