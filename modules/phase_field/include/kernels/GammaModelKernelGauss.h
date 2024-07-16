
#pragma once

#include "ADKernel.h"

class GammaModelKernelGauss : public ADKernel
{
public:
  static InputParameters validParams();

  GammaModelKernelGauss(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _L_AD;

  const ADMaterialProperty<Real> & _m;

  const ADMaterialProperty<Real> & _dgammadx;
  const ADMaterialProperty<Real> & _dgammady;
  const ADMaterialProperty<Real> & _dgammadz;

  const unsigned int _op_num;
  const std::vector<const ADVariableValue *> _vals;
};
