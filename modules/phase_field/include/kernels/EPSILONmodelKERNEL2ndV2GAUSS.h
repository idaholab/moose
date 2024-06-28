
#pragma once

#include "ADKernel.h"

class EPSILONmodelKERNEL2ndV2GAUSS : public ADKernel
{
public:
  static InputParameters validParams();

  EPSILONmodelKERNEL2ndV2GAUSS(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _L_AD;

  const ADMaterialProperty<Real> & _dmdxplus;
  const ADMaterialProperty<Real> & _dmdyplus;
  const ADMaterialProperty<Real> & _dmdzplus;

  const MaterialProperty<Real> & _F;
};
