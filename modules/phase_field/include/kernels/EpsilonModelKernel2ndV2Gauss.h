#pragma once

#include "ADKernel.h"

class EpsilonModelKernel2ndV2Gauss : public ADKernel
{
public:
  static InputParameters validParams();

  EpsilonModelKernel2ndV2Gauss(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  // Material property L.
  const ADMaterialProperty<Real> & _L_AD;

  // Negative derivatives of m.
  const ADMaterialProperty<RealGradient> & _dm_minus;

  // Base name of the free energy function.
  const MaterialProperty<Real> & _F;
};
