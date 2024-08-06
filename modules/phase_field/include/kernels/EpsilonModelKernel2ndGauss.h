#pragma once

#include "ADKernel.h"

class EpsilonModelKernel2ndGauss : public ADKernel
{
public:
  static InputParameters validParams();

  EpsilonModelKernel2ndGauss(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  // Material property L.
  const ADMaterialProperty<Real> & _L_AD;

  // Positive derivatives of m.
  const ADMaterialProperty<RealGradient> & _dm_plus;

  // Base name of the free energy function.
  const MaterialProperty<Real> & _F;
};
