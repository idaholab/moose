#pragma once

#include "AuxKernel.h"

class KLExpansionUserObject;

class KLNormalAux : public AuxKernel
{
public:
  static InputParameters validParams();
  KLNormalAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const KLExpansionUserObject & _kl_uo;
  const Real _mean;
};
