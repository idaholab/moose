
#pragma once

#include "ADKernel.h"

class EpsilonModelKernel2ndV2Gauss : public ADKernel
{
public:
  static InputParameters validParams();

  EpsilonModelKernel2ndV2Gauss(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _L_AD;

  const ADMaterialProperty<Real> & _dmdxplus;
  const ADMaterialProperty<Real> & _dmdyplus;
  const ADMaterialProperty<Real> & _dmdzplus;

  const MaterialProperty<Real> & _F;
};
