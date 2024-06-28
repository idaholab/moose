
#pragma once

#include "ADKernel.h"

class EPSILONmodelKERNEL2ndGAUSS : public ADKernel
{
public:
  static InputParameters validParams();

  EPSILONmodelKERNEL2ndGAUSS(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _L_AD;

  const ADMaterialProperty<Real> & _dmdx;
  const ADMaterialProperty<Real> & _dmdy;
  const ADMaterialProperty<Real> & _dmdz;

  const MaterialProperty<Real> & _F;
};
